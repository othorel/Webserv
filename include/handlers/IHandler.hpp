#ifndef IHANDLER_HPP
# define IHANDLER_HPP

# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Location.hpp"

class IHandler
{

	public :

		virtual HttpResponse handle(const HttpRequest &, const Location &) = 0;
		virtual ~IHandler();

};

#endif
