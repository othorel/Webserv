#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include <exception>
# include <map>
# include <string>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/config/Location.hpp"
# include "../../include/config/ServerConfig.hpp"

class ResponseBuilder
{
	private :

		HttpResponse _httpResponse;

		void buildRedirect(int code, const std::string & path);
		void buildError(int statusCode, const ServerConfig & server, const Location * location);
		
	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request, const ServerConfig & server);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		const HttpResponse & buildResponse(const HttpRequest& request, const ServerConfig & server);

};

static const Location & findMatchinglocation(
		const std::map<std::string, Location> & locations,
		const std::string & target);
static std::string selectErrorPage(int statusCode, const ServerConfig & server, const Location * location);

#endif
