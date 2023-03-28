#include <thread>
#include <iostream>
#include <functional>

#include "chat.h"
#include "../common/myexcept.h"
#include "client.h"
#include "io.h"
#include "protocol.h"
#include "logger.h"

namespace {
    // to automatically leave a room when go out of scope
    struct scoped_room_client 
    {
        scoped_room_client(room* r, const std::string& clientname) 
            : m_clientname(clientname), m_room(r) 
        {
        }

        ~scoped_room_client() 
        { 
            m_room->remove(m_clientname);
            m_room->send_to_all(
                "server", 
                protocol::response("server", std::string("**user ") + m_clientname + " left**"));

            logger::info(std::string("client ") + m_clientname + std::string(" disconnected"));
        }

        std::string m_clientname;
        room* m_room;
    };
}

///////////////////////////////////////////////////////////////////////////////////////////////////
chat::chat(in_port_t port)
    : m_sock()
    , m_global_room()
{
    // prepare everithing to be ready to accept incomming tcp connections
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        throw errno_exception(errno);
    }
    m_sock.reset(sock);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if(bind(m_sock.get(), (sockaddr*)&addr, sizeof(addr)) == -1) {
        throw errno_exception(errno);
    }

    if(listen(m_sock.get(), 5) == -1) {
        throw errno_exception(errno);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void chat::run()
{
    // the main loop of chat server
    // accept incomming connections
    for(;;) {
        int client;
        if( (client = accept(m_sock.get(), NULL, NULL)) == -1) {
            throw errno_exception(errno);
        }

        // here we use thread-per-client technic
        // does not scale well but simple to write and understand 
        std::thread( std::bind(&chat::handle_client, this, std::placeholders::_1), client ).detach();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void chat::handle_client(int client_sock)
try
{
    // RAII for the socket
    scoped_socket client_ss(client_sock);
    
    // wait for client to properly connect and send us his/her name
    auto p_client = wait_client_connected(client_sock); 
    
    // in case of error we just exit
    if(!p_client) {
        return;
    }
    auto client_name = p_client->name();

    // RAII for room membership
    scoped_room_client sru(&m_global_room, client_name);

    // here we use select to wait for two things:
    // 1) incomming requests from the client
    // 2) new messages to be sent to the client
    fd_set rset;
    FD_ZERO(&rset);
    for(;;) {
        FD_SET(client_sock, &rset);
        FD_SET(p_client->notifyfd(), &rset);
        int maxfdp1 = std::max(client_sock, p_client->notifyfd()) + 1;

        select(maxfdp1, &rset, NULL, NULL, NULL);

        // if there is a message from the client - block and read it
        if(FD_ISSET(client_sock, &rset)) {
            auto input = io::read(client_sock);
            
            if(!input)  // client disconnected
                break;

            // we have a request from the client - parse and execute
            handle_client_req(client_name, input.value());
        }

        // we are notified that there are messages to send to our client
        if(FD_ISSET(p_client->notifyfd(), &rset)) {
            // just reset eventfd to zero
            uint64_t not_used;
            if(read(p_client->notifyfd(), &not_used, sizeof(not_used)) == -1)
                throw errno_exception(errno);

            // loop until there is nothing to send
            std::string msg;
            while(p_client->pop_msg(msg)) {
                io::write(client_sock, msg);
            }
        }
    }
}
catch(const std::exception& e) {
    logger::info(e.what());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
pclient chat::wait_client_connected(int client_sock) {
    auto input = io::read(client_sock);
    
    // client decided to leave
    if(!input)
        return nullptr;

    // we expect the first message from the client to be the "connect" request
    auto req = protocol::request(input.value());
    if(req.get_type() != protocol::request::type::connect) {
        logger::info("unknown request");
        io::write(client_sock, "Connect request expected. You are dissconnected from the server");
        return nullptr;
    }

    auto client_name = req.to_connect_req().client_name();

    // now we are trying to add the new client to the global chatroom
    // it could fail if a user with the same name already exists in the room
    auto p_client = std::make_shared<client>(client_name);
    if(auto err = m_global_room.add(p_client); err) {
        io::write(client_sock, err.value());
        logger::info(err.value());
        return nullptr;
    }

    logger::info(std::string("client ") + client_name + std::string(" successfully connected"));

    m_global_room.send_to_all("server", 
        protocol::response("server", std::string("**user ") + p_client->name() + " joined**"));

    return p_client;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void chat::handle_client_req(const std::string& client_name, const std::string& client_req) 
{
    auto req = protocol::request(client_req);

    switch(req.get_type()) {
        case protocol::request::type::message: {
            auto message_req = req.to_message_req();
            if(message_req.to_client_name() == "all")
                m_global_room.send_to_all(
                    client_name, protocol::response(client_name, message_req.msg()) );
            else
                m_global_room.send_to_client(
                    message_req.to_client_name(), protocol::response(client_name, message_req.msg()) );
            break;
        }
        case protocol::request::type::all_clients: {
            std::string client_list;
            for(const auto& c : m_global_room.clients()) {
                client_list += c;
                client_list += " ";
            }
            m_global_room.send_to_client(
                client_name, protocol::response("server", client_list));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////