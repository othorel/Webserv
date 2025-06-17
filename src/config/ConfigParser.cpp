#include "../../include/config/ConfigParser.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/config/ServerConfig.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::ConfigParser(const std::string & filepath) {
	parsefile(filepath);
}

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
//Utils
int toInt(std::string value) {
	int result;
	char c;
	std::istringstream iss(value);
	
	if (!(iss >> result))		
		return (-1);
	if (iss >> c)
		return (-1);
	return (result);
}

std::string toString(int n) {
	std::ostringstream oss;
	oss << n;
	return (oss.str());
}

std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	size_t end = s.find_last_not_of(" \t\r\n");
	if (start == std::string::npos)
		return ("");
	return (s.substr(start, end - start + 1));
}
//Validation
void ConfigParser::validateListen(const std::string& ip, const std::string& port) {
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

void ConfigParser::validateServerNames(const std::vector<std::string>& names) {
	if (names.empty())
		std::cout << "No server_name defined (this server will act as the default)." << std::endl;
}

void ConfigParser::validateRoot(const std::string& root) {
	if (access(root.c_str(), F_OK) != 0)
		throw ValidationException("Root path not accessible: " + root);
}

void ConfigParser::validateErrorPage(const std::string& code, const std::string& path, const std::string& root) {
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

size_t parseSizeWithUnit(const std::string& value) {
	if (value.empty())
		throw std::runtime_error("Client max body size value is empty");
	char unit = value[value.size() - 1];
	size_t mul = 1;
	std::string numberPart = value;

	if (!isdigit(unit)) {
		numberPart = value.substr(0, value.size() - 1);
		if (unit == 'k' || unit == 'K')
			mul = 1024;
		else if (unit == 'm' || unit == 'M')
			mul = 1024 * 1024;
		else
			throw std::runtime_error("Invalid size unit for client max body size: " + value);
	}
	int number;
	std::istringstream iss(numberPart);
	iss >> number;
	if (iss.fail())
		throw std::runtime_error("Invalid number for clien max body size" + value);
	if (number <= 0)
		throw std::runtime_error("Client max body size must be greater than zero");
	return (static_cast<size_t>(number) * mul);
}

void	ConfigParser::debug() const
{
		std::cout << "Nombre de serveurs parsés : " << _serverConfigVector.size() << std::endl;

		for (size_t i = 0; i < _serverConfigVector.size(); ++i)
		{
			_serverConfigVector[i].printServerConfig(i);
		}

		std::cout << "\nValidation réussie : la configuration est correcte." << std::endl;
}

//Parser
void ConfigParser::parsefile(const std::string& filepath) {
	std::ifstream file(filepath.c_str());
	if (!file)
		throw ParseException("Cannot open config file");

	std::string line;
	bool inServer = false;
	bool inLocation = false;
	//server
	std::pair<int, std::string> listen;
	std::vector<std::string> server_names;
	std::string root;
	std::map<int, std::string> error_pages;
	std::map<std::string, Location> locations;
	size_t client_max_body_size = 1024 * 1024;
	//bonus server
	std::string session_name;
	int session_timeout = 0;
	bool session_enable = false;
	//location
	std::string loc_path;
	std::vector<std::string> loc_methods;
	std::map<int, std::string> loc_error_pages;
	std::string loc_upload_path;
	std::string loc_root;
	std::string loc_index;
	std::string loc_redirect_path;
	int loc_redirect_code = 0;
	bool loc_has_redirect = false;
	bool loc_autoindex = false;
	//bonus location
	std::vector<std::string> loc_cgi_extension;
	bool loc_cookies_enabled = false;

	while (std::getline(file, line)) {
		line = trim(line);
		size_t comment_pos = line.find('#');
		if (comment_pos != std::string::npos)
			line = line.substr(0, comment_pos);
		line = trim(line);
		if (line.empty())
			continue;
		if (line != "server {" && line != "}" && !(line.find("location") == 0 && line.find('{') != std::string::npos)) {
			size_t semicolon_pos = line.find(';');
			if (semicolon_pos == std::string::npos)
				throw ParseException("Missing ';' at end of directive: " + line);
			std::string after_semicolon = trim(line.substr(semicolon_pos + 1));
			if (!after_semicolon.empty())
				throw ParseException("Unexpected text after ';': " + after_semicolon + "'");
			line = trim(line.substr(0, semicolon_pos));
		}
		if (line == "server {") {
			if (inServer)
				throw ParseException("Nested server block is not allowed.");
			inServer = true;
			listen = std::make_pair(0, "");
			server_names.clear();
			root.clear();
			error_pages.clear();
			locations.clear();
			client_max_body_size = 1024 * 1024;
			session_name.clear();
			session_timeout = 0;
			session_enable = false;
			continue;
		}
		if (line == "}") {
			if (inLocation) {
				if (loc_root.empty())
					loc_root = root;
				Location loc(loc_path, loc_methods, loc_error_pages, loc_upload_path, loc_root, loc_index, loc_redirect_path, loc_redirect_code, loc_has_redirect, loc_autoindex, loc_cgi_extension, loc_cookies_enabled);
				locations[loc_path] = loc;
				inLocation = false;
			}
			else if (inServer) {
				ServerConfig server(listen, server_names, root, error_pages, locations, client_max_body_size, session_name, session_timeout, session_enable);
				validateServerNames(server.getServerNames());
				validateRoot(server.getRoot());
				std::map<int, std::string> eps = server.getErrorPages();
				for (std::map<int, std::string>::const_iterator it = eps.begin(); it != eps.end(); ++it)
					validateErrorPage(toString(it->first), it->second, server.getRoot());

				std::map<std::string, Location> locs = server.getLocations();
				for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
					const Location& loc = it->second;
					validateMethods(loc.getMethods());
					validateRoot(loc.getRoot());
				}
				_serverConfigVector.push_back(server);
				inServer = false;
			}
			continue;
		}
		if (line.find("location") == 0 && line.find("{") != std::string::npos) {
			if (!inServer)
				throw ParseException("Location block outside of server block");
			inLocation = true;
			std::istringstream iss(line);
			std::string keyword;
			iss >> keyword >> loc_path;
			loc_methods.clear();
			loc_upload_path.clear();
			loc_root.clear();
			loc_index.clear();
			loc_redirect_path.clear();
			loc_cgi_extension.clear();
			loc_error_pages.clear();
			loc_redirect_code = 0;
			loc_has_redirect = false;
			loc_autoindex = false;
			loc_cookies_enabled = false;
			continue;
		}
		std::istringstream iss(line);
		std::string key;
		iss >> key;
		std::string value;
		std::getline(iss, value);
		value = trim(value);
		if (!value.empty() && value[value.size() - 1] == ';')
  			value.erase(value.size() - 1, 1);
		if (inLocation) {
			if (key == "methods" || key == "allowed_methods") {
				std::istringstream iss(value);
				std::string method;
				while (iss >> method)
					loc_methods.push_back(method);
			}
			else if (key == "upload_path") {
				loc_upload_path = value;
			}
			else if (key == "cgi_extension") {
				std::istringstream iss(value);
				std::string ext;
				while (iss >> ext)
					loc_cgi_extension.push_back(ext);
			}
			else if (key == "root") {
				loc_root = value;
			}
			else if (key == "index") {
				loc_index = value;
			}
			else if (key == "autoindex") {
				std::string loc_autoindex_str;
				validateAutoIndex(value);
				loc_autoindex_str = value;
				loc_autoindex = (value == "on");
			}
			else if (key == "return" || key == "redirect") {
				std::istringstream iss(value);
				iss >> loc_redirect_code >> loc_redirect_path;
				if (loc_redirect_code < 300 || loc_redirect_code > 399)
					throw ValidationException("Invalid redirection code: " + toString(loc_redirect_code));
				loc_has_redirect = true;
			}
			else if (key == "cookies_enabled")
				loc_cookies_enabled = (value == "on");
			else if (key == "error_page") {
				std::istringstream iss(value);
				int code;
				std::string path;
				if (!(iss >> code))
					throw ParseException("Missing error code in error_page directive");
				if (!(iss >> path))
					throw ParseException("Missing path in error_page directive for code: " + toString(code));
				error_pages[code] = path;
			}
		}
		else if (inServer) {
			if (key == "listen") {
   				size_t sep = value.find(':');
    			if (sep != std::string::npos) {
        			std::string ip = value.substr(0, sep);
					std::string port_str = value.substr(sep + 1);
        			int port = toInt(port_str);
        			validateListen(ip, port_str);
        			listen = std::make_pair(port, ip);
    			} 
				else {
					std::string port_str = value;
					int port = toInt(port_str);
            		validateListen("0.0.0.0", port_str);
					listen = std::make_pair(port, "0.0.0.0");
    			}
			}
			else if (key == "server_name") {
				std::istringstream iss(value);
				std::string name;
				while (iss >> name)
					server_names.push_back(name);
			}
			else if (key == "root") {
				root = value;
			}
			else if (key == "error_page") {
				std::istringstream iss(value);
				int code;
				std::string path;
				if (!(iss >> code))
					throw ParseException("Missing error code in error_page directive");
				if (!(iss >> path))
					throw ParseException("Missing path in error_page directive for code: " + toString(code));
				error_pages[code] = path;
			}
			else if (key == "client_max_body_size")
				client_max_body_size = parseSizeWithUnit(value);
			else if (key == "session_name")
				session_name = value;
			else if (key == "session_timeout") {
				session_timeout = toInt(value);
				if (session_timeout <= 0)
					throw ValidationException("Invalid session timeout value: " + value);
			}
			else if (key == "session_enable") {
				if (value != "on" && value != "off")
					throw ValidationException("Session enable must be 'on' or 'off'");
				session_enable = (value == "on");
			}
		}
	}
	if (inLocation)
		throw ParseException("Unexpected end of file: missing closing '}' for location block");
	if (inServer)
		throw ParseException("Unexpected end of file: missing closing '}' for server block");
	if (_serverConfigVector.empty())
		throw ParseException("No server block defined in config file");
}
