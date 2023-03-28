#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <iostream>
#include "logger.h"

class Log {
public:
    Log() 
    : m_thread()
    , m_mut()
    , m_cv()
    , m_entries()
    , m_stop(false)
    {
        m_thread = std::thread( 
            [this]() {
                std::unique_lock lk(m_mut);

                while(true) {
                    m_cv.wait(lk, [this]() { return m_stop || !m_entries.empty(); });
                    if(m_stop)
                        break;
                    
                    std::queue<std::string> entries{};
                    entries.swap(m_entries);

                    lk.unlock();
                    while(!entries.empty()) {
                        std::cout << entries.front() << std::endl;
                        entries.pop();
                    }
                    lk.lock();
                }
            } 
        );
    }

    ~Log()
    {
        {
            std::lock_guard l(m_mut);
            m_stop = true;
        }
        m_cv.notify_one();
        m_thread.join();
    }

    void info(const std::string& str)
    {
        {
            std::lock_guard l(m_mut);
            m_entries.push(str);
        }
        m_cv.notify_one();
    }

    void error(const std::string& str) 
    {
        std::string message = std::string("error: ") + str;
        {
            std::lock_guard l(m_mut);
            m_entries.push(message);
        }
        m_cv.notify_one();
    }

private:
    std::thread m_thread;
    std::mutex m_mut;
    std::condition_variable m_cv;
    std::queue<std::string> m_entries;
    bool m_stop;
};

Log& log_object() {
    static Log l{};
    return l;
}

void logger::info(const std::string& str)
{
    log_object().info(str);
}

void logger::error(const std::string& str)
{
    log_object().error(str);
}