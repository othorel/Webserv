#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <map>
# include <cstring>
# include "Location.hpp"

class ServerConfig {

	private:

		std::pair<int, std::string> _listen;
		std::vector<std::string> _server_names;
		std::string _root;
		std::map<int, std::string> _error_pages;
		std::map<std::string, Location> _locations;

	public:

		ServerConfig(
			std::pair<int, std::string> listen,
			std::vector<std::string> _server_names,
			std::string root,
			std::map<int, std::string> error_pages,
			std::map<std::string, Location> locations
		);
		~ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);

		const std::pair<int, std::string>& getListen() const;
		const std::vector<std::string>& getServerNames() const;
		const std::string& getRoot() const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::map<std::string, Location>& getLocations() const;
};

#endif