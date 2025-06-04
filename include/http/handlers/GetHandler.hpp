#ifndef GETHANDLER_HPP
# define GETHANDLER_HPP

# include "AHandler.hpp"
# include "../HttpRequest.hpp"
# include "../HttpResponse.hpp"
# include "../../config/Location.hpp"

class GetHandler : public AHandler
{
	private :
	
		GetHandler();
		GetHandler(const GetHandler & other);
		GetHandler & operator=(const GetHandler & other);
		~GetHandler();

	public :
	
		HttpResponse handle(const HttpRequest & request, const Location & location, const ServerConfig & server);

};

#endif
