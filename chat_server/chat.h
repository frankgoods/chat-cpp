#ifndef __CHAT_H__
#define __CHAT_H__

#include <sys/socket.h>
#include <arpa/inet.h>
#include "room.h"
#include "client.h"
#include "../common/mysocket.h"

class chat {
public:
    chat(in_port_t port);

    void run();

private:
    void handle_client(int client_sock);
    pclient wait_client_connected(int client_sock);
    void handle_client_req(const std::string& client_name, const std::string& cliend_msg);

private:
    scoped_socket m_sock;
    room m_global_room;
};

#endif