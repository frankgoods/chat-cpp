#include <iostream>
#include <unistd.h>

#include "io.h"
#include "../common/myexcept.h"
#include "logger.h"

const int BUFSIZE = 100;

///////////////////////////////////////////////////////////////////////////////////////////////////
std::optional<std::string> io::read(int socket)
{
    char buf[BUFSIZE];

    ssize_t total_bytes = 0;
    ssize_t read_bytes = 0;
    char byte; 
    while(read_bytes = ::read(socket, &byte, 1)) {
        if(read_bytes == -1) {
            throw errno_exception(errno);
        }

        if(read_bytes == 0 || byte == '\n') {
            buf[total_bytes] = '\0';
            return std::string(buf);
        }

        if(total_bytes < BUFSIZE - 1) {
            buf[total_bytes] = byte;
            ++total_bytes;
        }
    }

    return std::nullopt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void io::write(int socket, std::string data)
{
    ssize_t sent_bytes;
    sent_bytes = ::write(socket, data.c_str(), data.size() + 1);

    if(sent_bytes == -1)
        throw errno_exception(errno);

    if(sent_bytes < data.size() + 1)
        logger::error("failed to sent entire msg to a client");

}
