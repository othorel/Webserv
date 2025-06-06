#ifndef POSTHANDLER_HPP
# define POSTHANDLER_HPP

# include "AHandler.hpp"
# include "../HttpRequest.hpp"
# include "../HttpResponse.hpp"
# include "../../config/Location.hpp"

class PostHandler : public AHandler
{
	public :
	
		PostHandler();
		PostHandler(const PostHandler & other);
		PostHandler & operator=(const PostHandler & other);
		~PostHandler();

	public :
	
		HttpResponse handle(const HttpRequest & request, const Location & location, const ServerConfig & server);

};

#endif
