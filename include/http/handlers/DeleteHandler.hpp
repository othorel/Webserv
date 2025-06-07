#ifndef DELETHANDLER_HPP
# define DELETHANDLER_HPP

# include "AHandler.hpp"
# include "../HttpRequest.hpp"
# include "../HttpResponse.hpp"
# include "../../config/Location.hpp"

class DeleteHandler : public AHandler
{
	public :
	
		DeleteHandler();
		DeleteHandler(const DeleteHandler & other);
		DeleteHandler & operator=(const DeleteHandler & other);
		~DeleteHandler();

	public :
	
		HttpResponse handle(const HttpRequest & request, const Location & location, const ServerConfig & server);

};

static void checkDeleteValidity(const std::string & path);

#endif
