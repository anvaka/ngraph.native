#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

class base_exception
    : public std::runtime_error
{
public:
    explicit base_exception(
        const std::wstring& _Message)
        : std::runtime_error("base_exception")
        , _msg(_Message)
    {
    }

    std::wstring get_message() const
    {
        return _msg;
    }

private:
    std::wstring _msg;
};

#define THROW_EXCEPTION(msg) \
{ \
    std::wstringstream wss; \
    wss << msg; \
    throw base_exception(wss.str()); \
}
