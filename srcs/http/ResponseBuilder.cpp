#ifndef RESPONSE_BUILDER
# define RESPONSE_BUILDER

# include <exception>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/Route.hpp"

class ResponseBuilder
{
	private :

		HttpResponse _httpResponse;

	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request, const Route& route);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		HttpResponse buildResponse(const HttpRequest& request, const Route& route);
		HttpResponse buildError(int statusCode);
		const HttpResponse & getResponse() const;

		class HttpErrorException : public std::exception
		{
			private:

				int _statusCode;

			public:
			
				HttpErrorException(int code) :
					_statusCode(code)
				{}

				int getStatusCode() const
				{
					return (_statusCode);
				}

				const char* what() const throw()
				{
					return ("HTTP Error Exception");
				}
			};

};

#endif





# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/http/Route.hpp"

ResponseBuilder::ResponseBuilder() :
	_httpResponse()
{}

ResponseBuilder::ReponseBuilder(const HttpRequest& request, const Route& route)
{
	_httpResponse()
}

ResponseBuilder(const ResponseBuilder & other);
ResponseBuilder & operator=(const ResponseBuilder & other);
~ResponseBuilder();





