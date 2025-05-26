#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <vector>
# include <fstream>
# include <sstream>
# include <cstdlib>
# include "ServerConfig.hpp"

class ConfigParser {

	private:

		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);

	public:

		ConfigParser();
		~ConfigParser();
		static std::vector<ServerConfig> parse(const std::string& filename);
};

#endif