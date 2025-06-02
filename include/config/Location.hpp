#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <vector>

class Location {

	private:

		Location(const Location& other);
		Location& operator=(const Location& other);

	public:

		Location();
		~Location();
		std::string path;
		std::vector<std::string> methods;
		std::string upload_path;
		std::string cgi_extension;
};

#endif