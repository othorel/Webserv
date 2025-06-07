#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

# include <string>
# include <map>

class HttpRequest
{
	private :

		std::string							_method;
		std::string							_target;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		std::string							_body;
		size_t								_contentLength;
		
	public :

		HttpRequest();
		HttpRequest(const std::string & method,
					const std::string & uri,
					const std::string & version,
					const std::map<std::string, std::string> & headers,
					const std::string & body);
		HttpRequest(const HttpRequest & other);
		HttpRequest & operator=(const HttpRequest & other);
		~HttpRequest();

		const std::string & getMethod() const;
		const std::string & getTarget() const;
		const std::string & getVersion() const;
		const std::string & getBody() const;
		const std::map<std::string, std::string>  & getHeaders() const;
		std::string getHeaderValue(const std::string & key) const;
		bool		hasHeader(const std::string & key) const;
		size_t		getContentLength() const;
		size_t		getCurrentBodyLength() const;
		size_t		getMissingBodyLength() const;
		bool		BodyIsComplete() const;
		size_t		AppendBody(const std::string & buffer);
		void		debug() const;

};

#endif
