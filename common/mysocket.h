#ifndef __MYSOCKET_H__
#define __MYSOCKET_H__

#include <unistd.h>

class scoped_socket {
public:
    scoped_socket(int socket) : m_socket(socket) {}
    scoped_socket() : m_socket(-1) {}

    void reset(int socket)
    {
        relase();
        m_socket = socket;
    }

    int get() const 
    {
        return m_socket;
    }

    ~scoped_socket() 
    { 
        relase();
    }

    void relase()
    {
        if(m_socket != -1)
            close(m_socket); 
    }

    scoped_socket(const scoped_socket&) = delete;
    scoped_socket& operator=(const scoped_socket&) = delete;

private:
    int m_socket;
};

#endif