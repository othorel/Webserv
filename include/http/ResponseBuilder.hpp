#ifndef RESPONSEBUILDER_HPP
# define RESPONSEBUILDER_HPP

# include <exception>
# include <map>
# include "../../include/http/HttpRequest.hpp"
# include "../../include/http/HttpResponse.hpp"
# include "../../include/config/Location.hpp"

class ResponseBuilder
{
	private :

		HttpResponse _httpResponse;

		const Location & findMatchingRoute(
				const std::map<std::string, Location> & routes,
				const std::string & target) const;
		void buildRedirect(int code, const std::string & path);

	public :

		ResponseBuilder();
		ResponseBuilder(const HttpRequest& request,
						const std::map<std::string, Location> & routes);
		ResponseBuilder(const ResponseBuilder & other);
		ResponseBuilder & operator=(const ResponseBuilder & other);
		~ResponseBuilder();

		const HttpResponse & buildResponse(
				const HttpRequest& request,
				const std::map<std::string, Location> & routes);
		const HttpResponse & buildError(int statusCode);
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
