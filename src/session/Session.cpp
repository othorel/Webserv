#include"../../include/session/Session.hpp"

Session::Session() {}

Session::~Session() {}

void Session::setSession(const std::string& key, const std::string& value) {
    _data[key] = value;
}

std::string Session::getSession(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _data.find(key);
    if (it != _data.end())
        return (it->second);
    return ("");
}

bool Session::hasSession(const std::string& key) const {
    return (_data.find(key) != _data.end());
}

void Session::removeSession(const std::string& key) {
    _data.erase(key);
}
