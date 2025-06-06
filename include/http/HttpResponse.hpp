#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include <string>
# include <map>

class HttpResponse
{

	private :

		std::string							_version;
		int									_statusCode;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

	public :

		HttpResponse();
		HttpResponse(
				const std::string & version,
				int statusCode,
				const std::map<std::string, std::string> & headers,
				const std::string & body);
		HttpResponse(const HttpResponse & other);
		HttpResponse & operator=(const HttpResponse & other);
		~HttpResponse();

		const std::string & getVersion() const;
		int getStatusCode() const;
		const std::map<std::string, std::string> & getHeaders() const;
		const std::string & getBody() const;
		void addHeader(const std::string & key, const std::string & value);
		std::string toRawString() const;

};

#endif
