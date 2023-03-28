#include "room.h"

room::error_t room::add(pclient p_client) 
{
    std::lock_guard l(m_mut);

    if( m_clients.count(p_client->name()) ) {
        return std::string("user already exists");
    }

    m_clients[p_client->name()] = p_client;
    return std::nullopt;
}

void room::remove(const std::string& clientname) {
    std::lock_guard l(m_mut);
    m_clients.erase(clientname);
}

std::vector<std::string> room::clients() const {
    std::lock_guard l(m_mut);

    std::vector<std::string> result;
    result.reserve(m_clients.size());
    for(const auto &e : m_clients) {
        result.push_back(e.first);
    }

    return result;
}

void room::send_to_client(const std::string& to_clientname, const std::string& msg) {
    std::lock_guard l(m_mut);

    if(auto it = m_clients.find(to_clientname); it != m_clients.end()) {
        it->second->push_msg(msg);
    }
}

void room::send_to_all(const std::string& from_clientname, const std::string& msg) {
    std::lock_guard l(m_mut);
    for(const auto &c : m_clients) {
        if(c.first != from_clientname) // do not send to yourself
            c.second->push_msg(msg);
    }
}