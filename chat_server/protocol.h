#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <string>

namespace protocol{

    // list of possible requests from a client

    // request to connect to chat server
    class connect_req {
        public:
            connect_req(const std::string& client_name) : m_name(client_name) {}

            std::string client_name() const { return m_name; }
        private:
            std::string m_name;
    };

    // request to send a message (to someone or to everyon)
    class message_req {
        public:
            message_req(const std::string& rest_msg) {
                auto meta_end_pos = rest_msg.find('#');
                if(meta_end_pos == std::string::npos) {
                    return;
                }

                m_to_client = rest_msg.substr(0, meta_end_pos);
                m_msg = rest_msg.substr(meta_end_pos + 1);
            }

            std::string to_client_name() const { return m_to_client; }
            std::string msg() const { return m_msg; }
        private:
            std::string m_to_client;
            std::string m_msg;
    };

    // request to get all the connected clients from chat server 
    class clients_req {
    };

    // class represents a generic request from a client
    // it should be in the form of one of the available commands
    // listed above 
    class request {
        public:
            enum class type{connect, message, add_to_group, rem_from_group, all_clients, unknown};

        public:
            request(const std::string& msg) : m_msg(msg) {
                m_cmd_end_pos = msg.find('#');
                if(m_cmd_end_pos == std::string::npos) {
                    m_type = type::unknown;
                    return;
                }

                auto cmd = msg.substr(0, m_cmd_end_pos);
                if(cmd == "connect")
                    m_type = type::connect;
                else if(cmd == "message")
                    m_type = type::message;
                else if(cmd == "clients")
                    m_type = type::all_clients;
                else
                    m_type = type::unknown;
            }

            type get_type() const { return m_type; };

            connect_req to_connect_req() const { 
                return connect_req(m_msg.substr(m_cmd_end_pos + 1));
            };

            message_req to_message_req() const {
                return message_req(m_msg.substr(m_cmd_end_pos + 1));
            }

            clients_req to_clients_req() const {
                return clients_req{};
            }

        private:
            std::string m_msg;
            size_t m_cmd_end_pos;
            type m_type;
    };

    // make a response from the chat server to a client
    std::string response(const std::string& from_clientname, const std::string& message)
    {
        return from_clientname + std::string("#") + message;
    }
} // namespace protocol

#endif