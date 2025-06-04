#ifndef AHANDLER_HPP
# define AHANDLER_HPP

# include <string>
# include "../HttpRequest.hpp"
# include "../HttpResponse.hpp"
# include "../../config/ServerConfig.hpp"
# include "../../config/Location.hpp"

class AHandler
{

	public :

		virtual HttpResponse handle(const HttpRequest &, const Location &, const ServerConfig &) = 0;
		virtual ~AHandler();

	protected :

		std::string createIndexPath(std::string path, const Location & location);
		std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
		const std::string & selectRoot(const ServerConfig & server, const Location & location);

};

#endif
