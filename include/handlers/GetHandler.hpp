#ifndef GETHANDLER_HPP
# define GETHANDLER_HPP

# include "AHandler.hpp"
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Location.hpp"

class GetHandler : public AHandler
{

	public :

		GetHandler();
		GetHandler(const GetHandler & other);
		GetHandler & operator=(const GetHandler & other);
		~GetHandler();

		HttpResponse handle(const HttpRequest & request, const Location & location, const ServerConfig & server);

};

#endif
