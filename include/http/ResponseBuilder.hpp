#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include <exception>
# include <map>
# include <string>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/handlers/AHandler.hpp"
# include "../../include/config/Location.hpp"
# include "../../include/config/ServerConfig.hpp"

class ResponseBuilder
{
	private :

		HttpResponse _httpResponse;

		void buildRedirect(int statusCode, const ServerConfig & server, const Location * location);
		void buildError(int statusCode, const ServerConfig & server, const Location * location);
		void addMandatoryHeaders(const ServerConfig & server, size_t bodySize);
		
	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request, std::vector<ServerConfig> serverVector);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		const HttpResponse & buildResponse(const HttpRequest& request, std::vector<ServerConfig> serverVector);
		const HttpResponse & getHttpResponse() const;

};

static const Location & findMatchinglocation(
		const std::map<std::string, Location> & locations,
		const std::string & target);
static std::string selectErrorPage(int statusCode, const ServerConfig & server, const Location * location);
static const ServerConfig & selectServer(const HttpRequest& request, const std::vector<ServerConfig> & serverVector);
static std::string createAllowedMethodsList(const Location & location);
static AHandler * createGetHandler();
static AHandler * createPostHandler();

#endif
