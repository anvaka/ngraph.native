#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

class base_exception
    : public std::runtime_error
{
public:
    explicit base_exception(
        const std::string& _Message)
        : std::runtime_error("base_exception")
        , _msg(_Message)
    {
    }

    std::string get_message() const
    {
        return _msg;
    }

private:
    std::string _msg;
};

#define THROW_EXCEPTION(msg) \
{ \
    std::stringstream ss; \
    ss << msg; \
    throw base_exception(ss.str()); \
}
