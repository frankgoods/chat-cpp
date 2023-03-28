#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../common/myexcept.h"
#include "../common/mysocket.h"

const in_port_t PORT = 33333; 

int main() 
try
{
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock == -1) {
        throw errno_exception(errno);
    }

    scoped_socket srv_ss(client_sock);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);

    std::cout << "connecting to chat server socket..." << std::endl;
    if(connect(client_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
        throw errno_exception(errno);
    }

    std::string my_name;
    std::cout << "what is your name? ";
    std::cin >> my_name;

    std::cout << "sending connect request..." << std::endl;
    std::string connect_msg = std::string("connect#") + my_name + std::string("\n");
    ssize_t bytes_sent;
    if((bytes_sent = write(client_sock, connect_msg.data(), connect_msg.size())) == -1) {
        throw errno_exception(errno);
    }

    char buf[100];
    ssize_t read_bytes;
    
    std::cout << "getting reply from the server..." << std::endl;
    if((read_bytes = read(client_sock, buf, 100)) == -1) {
        throw errno_exception(errno);
    }
    buf[read_bytes] = '\0';
    std::cout << buf << std::endl;


    fd_set rset;
    int maxfdp1;
    bool stdin_eof = false;
    FD_ZERO(&rset);

    std::cout << "?: " << std::flush;
    for(;;) {
        FD_SET(client_sock, &rset);
        if(!stdin_eof) 
            FD_SET(fileno(stdin), &rset);

        maxfdp1 = std::max(client_sock, fileno(stdin)) + 1;

        select(maxfdp1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(client_sock, &rset)) {
            if((read_bytes = read(client_sock, buf, 100)) == -1) {
                throw errno_exception(errno);
            }
            if(read_bytes == 0) {
                std::cout << "Dissconnected from server" << std::endl;
                return 0;
            }

            buf[read_bytes] = '\0';
            std::cout << "\n" << buf << "\n?: " << std::flush;
        }

        if(FD_ISSET(fileno(stdin), &rset)) {
            if((read_bytes = read(fileno(stdin), buf, 100)) == -1) {
                throw errno_exception(errno);
            }

            if(read_bytes == 0) 
                stdin_eof = true;
            else {
                buf[read_bytes] = '\0';

                std::string msg;
                auto bufstr = std::string(buf);
                if(bufstr == "/all\n") {
                    msg = "clients#\n";
                } else {
                    std::string msg_to = "message#all#";

                    if(read_bytes > 1 && bufstr[0] == '@') {
                        auto nameEnd = bufstr.find(' ');
                        msg_to = std::string("message#") + bufstr.substr(1, nameEnd - 1) + "#";
                        bufstr = bufstr.substr(nameEnd +1);
                    }

                    msg = msg_to + bufstr + std::string("\n");
                }

                if((bytes_sent = write(client_sock, msg.data(), msg.size())) == -1) {
                    throw errno_exception(errno);
                }
                std::cout << "?: " << std::flush;   
            }
        }
    }
}
catch(const std::exception& e)
{
    std::cerr << e.what() << '\n';
}