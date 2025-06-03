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

		static void validateListen(const std::string& ip, const std::string& port);
		static void validateServerNames(const std::vector<std::string>& names);
		static void validateRoot(const std::string& root);
		static void validateErrorPage(const std::string& code, const std::string& path, const std::string& root);
		static void validateMethods(const std::vector<std::string>& methods);
		static void validateAutoIndex(const std::string& value);
		static void validateCgiPass(const std::string& cgi_pass);

	public:
	
		ConfigParser();
		ConfigParser(const std::string& filepath);
		~ConfigParser();
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);

		void parsefile(const std::string& filepath);
		const std::vector<ServerConfig>& getServerConfigVector() const;

	class ParseException : public std::exception {

		private:

			std::string _msg;

		public:

			ParseException(const std::string& msg) : _msg(msg) {}
			virtual ~ParseException() throw() {}
			virtual const char* what() const throw() {return _msg.c_str(); }
	};

	class ValidationException : public std::exception {

		private:

			std::string _msg;

		public:

			ValidationException(const std::string& msg) : _msg(msg) {}
			virtual ~ValidationException() throw() {}
			virtual const char* what() const throw() {return _msg.c_str(); }
	};
};

#endif