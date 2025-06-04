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
		void ResponseBuilder::buildError(int statusCode, const ServerConfig & server, const Location * location);
		
	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request, const ServerConfig & server);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		const HttpResponse & buildResponse(
			const HttpRequest& request, const ServerConfig & server);

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
