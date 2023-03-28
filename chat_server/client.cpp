#include <unistd.h>
#include <sys/socket.h>
#include <iostream>
#include <sys/eventfd.h>
#include "client.h"
#include "../common/myexcept.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
client::client(const std::string& client_name)
    : m_name(client_name)
{
    m_event_fd = eventfd(0, 0);
    if(m_event_fd == -1) {
        throw errno_exception(errno);
    }
}

void client::push_msg(const std::string& msg){
    {
        std::lock_guard l(m_mut);
        m_msgs.push(msg);
    }

    // notify that we have a new value in the queue
    uint64_t e = 1;
    if(write(m_event_fd, &e, sizeof(e)) == -1)
        throw errno_exception(errno);
}

bool client::pop_msg(std::string& msg){
    std::lock_guard l(m_mut);
    if(m_msgs.empty())
        return false;

    msg = m_msgs.front();
    m_msgs.pop();

    return true;
}

std::string client::name() const{
    return m_name;
}
int client::notifyfd() const{
    return m_event_fd;
}

///////////////////////////////////////////////////////////////////////////////////////////////////