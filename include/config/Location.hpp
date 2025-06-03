#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>
# include <algorithm>

class Location {

	private:
	
		std::string _path;
		std::vector<std::string> _methods;
		std::string _upload_path;
		std::string _cgi_extension;
		std::string _root;
		std::string _index;
		std::string _redirectPath;
		int _redirectCode;
		bool _hasRedirect;
		bool _autoindex;
	
	public:
	
		Location();
		Location(
			std::string path,
			std::vector<std::string> methods,
			std::string upload_path,
			std::string cgi_extension,
			std::string root,
			std::string index,
			std::string redirectPath,
			int redirectCode,
			bool hasRedirect,
			bool autoindex
		);
		~Location();
		Location(const Location& other);
		Location& operator=(const Location& other);

		const std::string& getPath() const;
		const std::vector<std::string>& getMethods() const;
		const std::string& getUploadPath() const;
		const std::string& getCgiExtension() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		const std::string& getRedirectPath() const;
		int getRedirectCode() const;

		bool hasRedirect() const;
		bool isAutoIndex() const;
		bool isValidMethod(const std::string& method) const;
};

#endif