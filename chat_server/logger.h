#ifndef __LOG_H__
#define __LOG_H__

#include <string>

namespace logger {
    void info(const std::string& str);
    void error(const std::string& str);
} // namespace logger

#endif