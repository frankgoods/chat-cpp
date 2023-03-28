#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <string>
#include <optional>
#include <memory>
#include <queue>
#include <mutex>

class client {
public:
    client(const std::string& client_name);

    void push_msg(const std::string& msg);
    bool pop_msg(std::string& msg);
    
    std::string name() const;
    int notifyfd() const;

private:
    std::string m_name;
    int m_event_fd;

    std::mutex m_mut;
    std::queue<std::string> m_msgs;
};
using pclient = std::shared_ptr<client>;

#endif