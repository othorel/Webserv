#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include <string>
# include <map>
# include <cstring>
# include <stdexcept>
# include <algorithm>
# include <iostream>
# include "Location.hpp"

class ServerConfig {

	private:

		std::pair<int, std::string> _listen;
		std::vector<std::string> _server_names;
		std::string _root;
		std::map<int, std::string> _error_pages;
		std::map<std::string, Location> _locations;
		size_t _client_max_body_size;
		bool _keepAlive;
		int _keepAliveTimeout;
		int _keepAliveMaxRequests;
		//bonus
		std::string _sessionName;
		int _sessionTimeout;
		bool _sessionEnable;

	public:

		ServerConfig();
		ServerConfig(
			std::pair<int, std::string> listen,
			std::vector<std::string> _server_names,
			std::string root,
			std::map<int, std::string> error_pages,
			std::map<std::string, Location> locations,
			size_t client_max_body_size,
			bool keepAlive,
			int keepAliveTimeout,
			int keepAliveMaxRequests,
			std::string sessionName,
			int sessionTimeout,
			bool sessionEnable
		);
		~ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);

		const std::pair<int, std::string>& getListen() const;
		const std::vector<std::string>& getServerNames() const;
		const std::string& getRoot() const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::map<std::string, Location>& getLocations() const;
		size_t getClientMaxBodySize() const;
		bool hasErrorPage(int code) const;
		const std::string& getErrorPage(int code) const;
		bool getKeepAlive() const;
		int getKeepAliveTimeout() const;
		int getKeepAliveMaxRequests() const;
		//bonus
		const std::string& getSessionName() const;
		int getSessionTimeout() const;
		bool isSessionEnable() const;
		bool hasServerName(const std::string & nameToFind) const;
		void printServerConfig(int index) const;
};

#endif