#include "UserInfo.h"

bool IsValidMessage(std::string_view message)
{
    if (message.empty())
        return false;

	// Only white-space
    if (message.find_first_not_of(" \t\n\v\f\r") == std::string::npos)
        return false;

	// too long = invalid
    if (message.size() > MaxMessageLength)
        return false;

    return true;
}

std::string TrimMessage(std::string_view message)
{
    if (message.size() > MaxMessageLength)
        return std::string(message.substr(0, MaxMessageLength));

    return std::string(message);
}