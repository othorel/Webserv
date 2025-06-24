#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>
# include <algorithm>
# include <map>
# include <stdexcept>

class Location {

	private:
	
		std::string _path;
		std::vector<std::string> _methods;
		std::map<int, std::string> _error_pages;
		std::string _upload_path;
		std::string _root;
		std::string _index;
		std::string _redirectPath;
		int _redirectCode;
		bool _hasRedirect;
		bool _autoindex;
		std::vector<std::string> _cgiExtensions;
		bool _cookiesEnabled;
		ssize_t _client_max_body_size;
	
	public:
	
		Location();
		Location(
			std::string path,
			std::vector<std::string> methods,
			std::map<int, std::string> error_pages,
			std::string upload_path,
			std::string root,
			std::string index,
			std::string redirectPath,
			int redirectCode,
			bool hasRedirect,
			bool autoindex,
			std::vector<std::string> cgiExtensions,
			bool cookiesEnabled,
			ssize_t client_max_body_size
		);
		~Location();
		Location(const Location& other);
		Location& operator=(const Location& other);

		const std::string& getPath() const;
		const std::vector<std::string>& getMethods() const;
		const std::string& getUploadPath() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		const std::string& getRedirectPath() const;
		const std::string& getErrorPage(int code) const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::vector<std::string>& getCgiExtensions() const;
		int getRedirectCode() const;
		ssize_t getClientMaxBodySize() const;
		bool hasRedirect() const;
		bool isAutoIndex() const;
		bool isValidMethod(const std::string& method) const;
		bool hasCgi() const;
		bool hasErrorPage(int code) const;
		bool isCookiesEnabled() const;
};

#endif