#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

class Location {

	private:
	
		std::string _path;
		std::vector<std::string> _methods;
		std::string _upload_path;
		std::string _cgi_extension;
		std::string _root;
		std::string _index;
		bool _autoindex;
	
	public:
	
		Location(
			std::string path,
			std::vector<std::string> methods,
			std::string upload_path,
			std::string cgi_extension,
			std::string root,
			std::string index,
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
		bool isAutoIndex() const;
};

#endif