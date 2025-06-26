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

		static bool isDirectory(const std::string & path);
		static bool isRegularFile(const std::string & path);
		static bool fileExists(const std::string & path);
		static std::string httpStatusMessage(int code);
		static int stringToInt(std::string string);
		static std::string getCurrentDate();
		static std::string getMimeType(const std::string & path);
		static std::string getExtensionFromMimeType(const std::string & mimeType);
		static bool hasWritePermission(const std::string & path);
		static std::string getUnixTimestampString();
		static std::string generateUniqueTimestamp();
		static void trimFinalSlash(std::string & string);
		static void trimSlashes(std::string & string);
		static void stringToLower(std::string & string);

		template<typename T>
		static std::string numberToString(T value)
		{
			std::ostringstream oss;
			oss << value;
			return (oss.str());
		}

};

#endif
