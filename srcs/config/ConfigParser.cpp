#include "ConfigParser.hpp"

static std::string trim(const std::string& str) {
	size_t first = str.find_first_not_of(" \t\n\r");
	size_t last = str.find_last_not_of(" \t\n\r");
	if (first == std::string::npos)
		return ("");
	return (str.substr(first, last - first + 1));
}

std::vector<ServerConfig> ConfigParser::parse(const std::string& filename) {

	std::ifstream file(filename.c_str());
	std::vector<ServerConfig> servers;
	std::string line;
	ServerConfig current_server;
	Location current_location;
	bool in_serv = false;
	bool in_location = false;

	while (std::getline(file, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
	}
}