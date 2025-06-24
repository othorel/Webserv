#include "../../include/config/ConfigValidator.hpp"
#include "../../include/config/ConfigParserUtils.hpp"
#include "../../include/config/ParseException.hpp"
#include "../../include/config/ValidationException.hpp"

void validateListen(const std::string& ip, const std::string& port) {
	size_t dot = 0;
	size_t start = 0;
	int count = 0;
	int x = toInt(port);

	if (x < 1 || x > 65535)
		throw ValidationException("Invalid port: " + port);
	while ((dot = ip.find('.', start)) != std::string::npos) {
		std::string part = ip.substr(start, dot - start);
		int n = toInt(part);
		if (n < 0 || n > 255)
			throw ValidationException("Invalid IP segment: " + part);
		start = dot + 1;
		count++;
	}
	std::string lastPart = ip.substr(start);
	int n = toInt(lastPart);
	if (n < 0 || n > 255)
		throw ValidationException("Invalid IP segment: " + lastPart);
	count++;
	if (count != 4)
		throw ValidationException("Invalid IP address: " + ip);
}

void validateServerNames(const std::vector<std::string>& names) {
	if (names.empty())
		std::cout << "No server_name defined (this server will act as the default)." << std::endl;
}

void validateRoot(const std::string& root) {
	if (access(root.c_str(), F_OK) != 0)
		throw ValidationException("Root path not accessible: " + root);
}

void validateErrorPage(const std::string& code, const std::string& path, const std::string& root) {
	int x = toInt(code);

	if (x < 100 || x > 599)
		throw ValidationException("Invalid HTTP error code: " + code);
	std::string fullPath = root;
	if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/' && !path.empty() && path[0] != '/')
		fullPath += '/';
	fullPath += path;
	if (access(fullPath.c_str(), F_OK) != 0)
		throw ValidationException("Error page not found: " + fullPath);
}

void validateMethods(const std::vector<std::string>& methods) {
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

void validateAutoIndex(const std::string& value) {
	if (value != "on" && value != "off")
		throw ValidationException("Autoindex must be 'on' or 'off'");
}

void validateCgiExtension(const std::vector<std::string>& extensions) {
	std::set<std::string> allowedExtensions;
	allowedExtensions.insert(".pl");
	allowedExtensions.insert(".py");
	allowedExtensions.insert(".php");
	for (std::vector<std::string>::const_iterator it = extensions.begin(); it != extensions.end(); it++) {
		const std::string& ext = *it;
		if (ext.empty() || ext[0] != '.')
			throw ValidationException("CGI extension must start with a dot: " + ext);
		if (allowedExtensions.find(ext) == allowedExtensions.end())
			throw ValidationException("Unsupported CGI extension: " + ext);
	}
}
