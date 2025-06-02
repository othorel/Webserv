#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <map>
# include "Location.hpp"

class ServerConfig {

	private:

		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);

	public:

		ServerConfig();
		~ServerConfig();
		int port;
		std::string server_name;
		std::string root;
		std::map<int, std::string> error_pages;
		std::map<std::string, Location> Locations;
};

#endif