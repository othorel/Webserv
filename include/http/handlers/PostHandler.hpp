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
	
		HttpResponse handle(const HttpRequest & request,
			const Location & location, const ServerConfig & server);

};

static void checkPostValidity(
	const HttpRequest & request, const Location & location ,
	const ServerConfig & server, const std::string & path);
static std::string createPostFileName(
	const HttpRequest & request, const ServerConfig & server, const std::string & path);
static std::string sanitizeFilenamePart(const std::string & input);

#endif
