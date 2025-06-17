#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include <exception>
# include <unistd.h>
# include <algorithm>
# include <iostream>
# include "ServerConfig.hpp"

class ConfigParser {

	private:

		std::vector<ServerConfig> _serverConfigVector;

	public:
	
		ConfigParser();
		ConfigParser(const std::string& filepath);
		~ConfigParser();
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);

		void parsefile(const std::string& filepath);
		const std::vector<ServerConfig>& getServerConfigVector() const;

		void	debug() const;
};

#endif