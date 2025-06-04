# include <string>
# include <fstream>
# include <sys/stat.h>
# include <dirent.h>
#include "../../include/http/HttpUtils.hpp"

/* ************************************************************************** */
/*                              static utils methods                          */
/* ************************************************************************** */

// Reads the file and returns a string with the data red
std::string HttpUtils::readFile(const std::string & path)
{
	std::ifstream file(path.c_str());
	if (!file) {
		throw std::runtime_error("Could not open file " + path); }
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return (content.str());
}

// return true if the file is a directory
bool HttpUtils::isDirectory(const std::string & path)
{
	struct stat s;
	if (!stat(path.c_str(), &s))
		return ((s.st_mode & S_IFMT) == S_IFDIR);
	return (false);
}

// Returns true if a file exists
bool HttpUtils::fileExists(const std::string & path)
{
	struct stat s;
	return (stat(path.c_str(), &s) == 0 && S_ISREG(s.st_mode));
}

// Function that returns an HTTP error message according to an HTTP error code
std::string httpStatusMessage(int code)
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
