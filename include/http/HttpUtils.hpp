#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

# include <string>
# include <sstream>

class HttpUtils
{
	private:

		HttpUtils();
		~HttpUtils();
		HttpUtils(const HttpUtils &);
		HttpUtils & operator=(const HttpUtils &);

	public:

		static std::string readFile(const std::string & path);
		static bool isDirectory(const std::string & path);
		static bool fileExists(const std::string & path);
		static std::string generateAutoIndex(const std::string & dirPath, const std::string & uriPath);
		static std::string httpStatusMessage(int code);
		static int stringToInt(std::string string);
		static std::string getCurrentDate();
		static std::string getMimeType(const std::string & path);

		template<typename T>
		static std::string numberToString(T value)
		{
			std::ostringstream oss;
			oss << value;
			return (oss.str());
		}

};

#endif
