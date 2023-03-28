#ifndef __ROOM_H__
#define __ROOM_H__

#include <optional>
#include <unordered_map>
#include <vector>
#include <mutex>

#include "client.h"

class room {
public:
    using error_t = std::optional<std::string>;

public:
    room() {}

    std::vector<std::string> clients() const;

    error_t add(pclient p_client);
    void remove(const std::string& clientname) ;
    void send_to_client(const std::string& to_clientname, const std::string& msg);
    void send_to_all(const std::string& from_clientname, const std::string& msg);

private:
    mutable std::mutex m_mut;
    std::unordered_map<std::string, pclient> m_clients;
};

#endif