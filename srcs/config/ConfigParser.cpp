#include "../../include/config/ConfigParser.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::~ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser& other) : _serverConfigVector(other._serverConfigVector) {}

ConfigParser& ConfigParser::operator=(const ConfigParser& other) {
	if (this != &other)
		_serverConfigVector = other._serverConfigVector;
	return (*this);
}

const std::vector<ServerConfig>& ConfigParser::getServerConfigVector() const {
	return (_serverConfigVector);
}

int toInt(std::string value) {
	int i;
	std::istringstream iss(value);
	iss >> i;
	if (iss && iss.eof())
		return (i);
	return (-1);
}

void ConfigParser::validateListen(const std::string& ip, const std::string& port) {
	size_t dot = 0;
	size_t start = 0;
	int count = 0;
	int x = toInt(port);

	if (x < 1 || x > 65535)
		throw ValidationException("Invalid x: " + port);
	while ((dot = ip.find('.', start)) != std::string::npos) {
		std::string part = ip.substr(start, dot - start);
		int n = atoi(part.c_str());
		if (n < 0 || n > 255)
			throw ValidationException("Invalid IP segment: " + part);
		start = dot + 1;
		count++;
	}
	if (count != 4)
		throw ValidationException("Invalid IP address: " + ip);
}

void ConfigParser::validateServerNames(const std::vector<std::string>& names) {
	if (names.empty())
		throw ValidationException("Server name list is empty");
}

void ConfigParser::validateRoot(const std::string& root) {
	if (access(root.c_str(), F_OK) != 0)
		throw ValidationException("Root path not accessible: " + root);
}

void ConfigParser::validateErrorPage(const std::string& code, const std::string& path, const std::string& root) {
	int x = toInt(code);

	if (x < 100 || x > 599)
		throw ValidationException("Invalid HTTP error code: " + code);
	std::string fullPath = root + path;
	if (access(fullPath.c_str(), F_OK) != 0)
		throw ValidationException("Error page not found: " + fullPath);
}

void ConfigParser::validateMethods(const std::vector<std::string>& methods) {
	const std::string validMethods[] = {"GET", "POST", "DELETE"};
	for (size_t i = 0; i < methods.size(); i++) {
		bool valid = false;
		for (int j = 0; j < 3; j++) {
			if (methods[i] == validMethods[j]) {
				valid = true;
				break;
			}
		}
		if (!valid)
			throw ValidationException("Invalid method: " + methods[i]);
	}
}

void ConfigParser::validateAutoIndex(const std::string& value) {
	if (value != "on" && value != "off")
		throw ValidationException("Autoindex must be 'on' or 'off'");
}

void ConfigParser::validateCgiPass(const std::string& cgi_pass) {
	if (access(cgi_pass.c_str(), X_OK) != 0)
		throw ValidationException("CGI script not executable: " + cgi_pass);
}
