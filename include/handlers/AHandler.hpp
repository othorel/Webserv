#ifndef AHANDLER_HPP
# define AHANDLER_HPP

# include <string>
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/ServerConfig.hpp"
# include "../config/Location.hpp"

class AHandler
{

	public :

		virtual HttpResponse handle(const HttpRequest &, const Location &, const ServerConfig &) = 0;
		virtual ~AHandler();

	protected :

		std::string numberToString(size_t value);
		std::string numberToString(int value);
		std::string readFile(const std::string & path);
		bool isDirectory(const std::string & path);
		bool fileExists(const std::string & path);
		std::string createIndexPath(std::string path, const Location & location);
		std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
		const std::string & selectRoot(const ServerConfig & server, const Location & location);

};

std::string httpStatusMessage(int code);

#endif
