#include <iostream>
#include "utils.h"
#include "chat.h"
#include "logger.h"

int main() 
try
{
    // if a tcp client is disconnected we do not want to get
    // SIGPIPE when performing  a write to that client socket
    // Error returned from the write operation is just enough
    utils::ignore_signal(SIGPIPE);

    const in_port_t PORT = 33333;
    chat ch(PORT);

    logger::info("Chat server listenning on port " + std::to_string(PORT));
    ch.run();
}
catch(const std::exception& e)
{
    logger::error( std::string(e.what()) + std::string("'\n") );
}

