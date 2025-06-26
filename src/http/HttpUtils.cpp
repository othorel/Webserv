#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include <unistd.h>
#include <sys/time.h>
#include "../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                              static utils methods                          */
/* ************************************************************************** */

// return true if the file is a directory
bool HttpUtils::isDirectory(const std::string & path)
{
	struct stat s;
	if (!stat(path.c_str(), &s))
		return ((s.st_mode & S_IFMT) == S_IFDIR);
	return (false);
}

bool HttpUtils::isRegularFile(const std::string & path)
{
	struct stat s;
	if (stat(path.c_str(), &s) != 0)
		return false;
	return S_ISREG(s.st_mode);
}

bool HttpUtils::fileExists(const std::string & path)
{
	struct stat s;
	if (stat(path.c_str(), &s) != 0)
		return false;
	return (S_ISREG(s.st_mode) || S_ISDIR(s.st_mode));
}

// Function that returns an HTTP error message according to an HTTP error code
std::string HttpUtils::httpStatusMessage(int code)
{
	switch (code)
	{
		// --- 1xx: Informational ---
		case 100: return "Continue";
		case 101: return "Switching Protocols";
		case 102: return "Processing";                      // RFC 2518
		case 103: return "Early Hints";                     // RFC 8297

		// --- 2xx: Success ---
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 203: return "Non-Authoritative Information";
		case 204: return "No Content";
		case 205: return "Reset Content";
		case 206: return "Partial Content";
		case 207: return "Multi-Status";                    // RFC 4918
		case 208: return "Already Reported";                // RFC 5842
		case 226: return "IM Used";                         // RFC 3229

		// --- 3xx: Redirection ---
		case 300: return "Multiple Choices";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 305: return "Use Proxy";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";              // RFC 7538

		// --- 4xx: Client Error ---
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 402: return "Payment Required";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 406: return "Not Acceptable";
		case 407: return "Proxy Authentication Required";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 411: return "Length Required";
		case 412: return "Precondition Failed";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 416: return "Range Not Satisfiable";
		case 417: return "Expectation Failed";
		case 418: return "I'm a teapot";                    // RFC 7168
		case 421: return "Misdirected Request";             // RFC 7540
		case 422: return "Unprocessable Entity";            // RFC 4918
		case 423: return "Locked";                          // RFC 4918
		case 424: return "Failed Dependency";               // RFC 4918
		case 425: return "Too Early";                       // RFC 8470
		case 426: return "Upgrade Required";
		case 428: return "Precondition Required";           // RFC 6585
		case 429: return "Too Many Requests";               // RFC 6585
		case 431: return "Request Header Fields Too Large"; // RFC 6585
		case 451: return "Unavailable For Legal Reasons";   // RFC 7725

		// --- 5xx: Server Error ---
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		case 506: return "Variant Also Negotiates";         // RFC 2295
		case 507: return "Insufficient Storage";            // RFC 4918
		case 508: return "Loop Detected";                   // RFC 5842
		case 510: return "Not Extended";                    // RFC 2774
		case 511: return "Network Authentication Required"; // RFC 6585

		default: return "Unknown Status";
	}
}

int HttpUtils::stringToInt(std::string string)
{
	int result = 0;
	std::stringstream ss;
	ss << string;
	ss >> result;
	return (result);
}

std::string HttpUtils::getCurrentDate()
{
	std::time_t now = std::time(NULL);
	std::tm *gmt = std::gmtime(&now);

	static const char* days[]  = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	std::ostringstream oss;
	oss << days[gmt->tm_wday] << ", ";
	oss << std::setw(2) << std::setfill('0') << gmt->tm_mday << " ";
	oss << months[gmt->tm_mon] << " ";
	oss << (gmt->tm_year + 1900) << " ";
	oss << std::setw(2) << std::setfill('0') << gmt->tm_hour << ":";
	oss << std::setw(2) << std::setfill('0') << gmt->tm_min << ":";
	oss << std::setw(2) << std::setfill('0') << gmt->tm_sec << " GMT";

	return (oss.str());
}

std::string HttpUtils::getMimeType(const std::string & path)
{
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) {
		return ("application/octet-stream"); }
	std::string extension = path.substr(dot +1);
	stringToLower(extension);

	//debug
	std::cout << "EXTENSION : " << extension << std::endl;

	static std::map<std::string, std::string> extensionMap;
	if (extensionMap.empty()) {
		extensionMap["html"] = "text/html";
		extensionMap["htm"] = "text/html";
		extensionMap["css"] = "text/css";
		extensionMap["txt"] = "text/plain";
		extensionMap["js"] = "application/javascript";
		extensionMap["jpg"] = "image/jpeg";
		extensionMap["jpeg"] = "image/jpeg";
		extensionMap["png"] = "image/png";
		extensionMap["gif"] = "image/gif";
		extensionMap["ico"] = "image/x-icon";
		extensionMap["json"] = "application/x-json";
		extensionMap["pdf"] = "application/pdf"; }
	
	if (extensionMap.find(extension) != extensionMap.end()) {
		return (extensionMap[extension]); }
	return ("application/octet-stream");
}

std::string HttpUtils::getExtensionFromMimeType(const std::string & mimeType)
{
	static std::map<std::string, std::string> mimeTypeMap;

	if (mimeTypeMap.empty()) {
		mimeTypeMap["text/html"] = "html";
		mimeTypeMap["text/css"] = "css";
		mimeTypeMap["text/plain"] = "txt";
		mimeTypeMap["application/javascript"] = "js";
		mimeTypeMap["image/jpeg"] = "jpg";
		mimeTypeMap["image/png"] = "png";
		mimeTypeMap["image/gif"] = "gif";
		mimeTypeMap["image/x-icon"] = "ico";
		mimeTypeMap["application/x-json"] = "json";
		mimeTypeMap["application/pdf"] = "pdf"; }

	if (mimeTypeMap.find(mimeType) != mimeTypeMap.end()) {
		return mimeTypeMap[mimeType]; }
	return "";
}

bool HttpUtils::hasWritePermission(const std::string & path)
{
	return (access(path.c_str(), W_OK) == 0);
}

std::string HttpUtils::getUnixTimestampString()
{
	std::time_t now = std::time(NULL);
	std::ostringstream oss;
	oss << now;
	return oss.str();
}

std::string HttpUtils::generateUniqueTimestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	std::ostringstream oss;
	oss << tv.tv_sec << "_" << tv.tv_usec;
	return oss.str();
}

void HttpUtils::trimFinalSlash(std::string & string)
{
	if (!string.empty() && string[string.size() - 1] == '/')
		string.erase(string.size() - 1);
}

void HttpUtils::trimSlashes(std::string & string)
{
	if (!string.empty() && string[0] == '/')
		string.erase(0, 1);
	if (!string.empty() && string[string.size() - 1] == '/')
		string.erase(string.size() - 1);
}

void HttpUtils::stringToLower(std::string & string)
{
	for (size_t i = 0; i < string.size(); ++i)
		string[i] = static_cast<char>(std::tolower(string[i]));
}
