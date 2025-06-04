#ifndef GETHANDLER_HPP
# define GETHANDLER_HPP

# include "IHandler.hpp"
# include "../http/HttpRequest.hpp"
# include "../http/HttpResponse.hpp"
# include "../config/Location.hpp"

class GetHandler : public IHandler
{

	public :

		HttpResponse handle(const HttpRequest &, const Location &);
		GetHandler();
		GetHandler(const GetHandler & other);
		GetHandler & operator=(const GetHandler & other);
		~GetHandler();

};

#endif
