#ifndef __IO_H__
#define __IO_H__

#include <optional>
#include <string>

namespace io {
    using read_result_t = std::optional<std::string>;
    read_result_t read(int socket);

    void write(int socket, std::string data);
}

#endif