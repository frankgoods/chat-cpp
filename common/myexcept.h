#ifndef __MYEXCEPT_H__
#define __MYEXCEPT_H__

#include <exception>
#include <string.h>

class errno_exception : public std::exception {
public:
    errno_exception(int errnum) : m_errnum(errnum) {}

    virtual const char*
    what() const noexcept { return strerror_r(m_errnum, m_buf, BUFLEN); } 

private:
    enum {BUFLEN = 100};
    int m_errnum;
    mutable char m_buf[BUFLEN];
};

#endif