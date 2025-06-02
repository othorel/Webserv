#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include "ServerConfig.hpp"

class ConfigParser {

	public:
	
		ConfigParser();
		~ConfigParser();
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		static std::vector<ServerConfig> parse(const std::string& filename);
};

#endif