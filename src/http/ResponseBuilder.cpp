#include "../../include/http/ResponseBuilder.hpp"
#include "../../include/http/HttpErrorException.hpp"
#include "../../include/process/ProcessRequest.hpp"
#include "../../include/http/HttpUtils.hpp"
#include "../../include/process/CookieHandler.hpp"

/* ************************************************************************** */
/*                              response builders                             */
/* ************************************************************************** */

void ResponseBuilder::buildResponse(ProcessRequest * process, int statusCode,
	const std::map<std::string, std::string> & headers, const std::string & body)
{
	if (!process)
		throw HttpErrorException(500, "in RB: Request is NULL.");
	process->_httpResponse = HttpResponse("HTTP/1.1", statusCode, headers, body);
	addFinalHeaders(process);
}

void ResponseBuilder::buildRedirect(ProcessRequest * process)
{
	if (!process)
		throw HttpErrorException(500, "in RB: Process is NULL.");

	std::string redirectPath = process->_location.getRedirectPath();
	int redirectCode = process->_location.getRedirectCode();

	std::string body =
		"<html><body><h1>" + HttpUtils::httpStatusMessage(redirectCode) + "</h1>\n"
		"<p>Redirecting to <a href=\"" + redirectPath + "\">" + redirectPath + "</a></p>\n"
		"</body></html>";

	std::map<std::string, std::string> headers;
	headers["content-type"] = "text/html";
	headers["location"] = redirectPath;
	headers["content-length"] = HttpUtils::numberToString(body.length());

	if (process->_file) {
		delete process->_file;
		process->_file = NULL;
	}
	buildResponse(process, redirectCode, headers, body);
}

void ResponseBuilder::errorBuilder(ProcessRequest * process, int statusCode, bool secondTime)
{
	if (!process)
		throw HttpErrorException(500, "in RB: Process is NULL.");

	if (process->_file) {
		delete process->_file;
		process->_file = NULL;
	}
	std::string errorPage = selectErrorPage(process, statusCode);
	std::map<std::string, std::string> headers;
	std::string mimeType = "text/html";
	std::string body = "";
	size_t bodyLen = 0;

	// if there is a file to read
	if (!errorPage.empty() && !secondTime) {
		std::string errorFilePath = createErrorFilePath(process, errorPage);
		try {
			process->_file = new File(errorFilePath);
		}
		catch (const std::bad_alloc&) {
			throw HttpErrorException(500, "in RB: Bad alloc.");
		}
		mimeType = process->_file->getMimeType();
		bodyLen = process->_file->getSize();
	}
	// there is no file
	else {
		body = "<html><body><h1>" + HttpUtils::numberToString(statusCode) + " " +
			HttpUtils::httpStatusMessage(statusCode) + "</h1>\n"
			"</body>\n"
			 "<footer><a href=\"/\">Home</a></footer>\n"
			"</html>";
		bodyLen = body.size();
	}
	headers["content-type"] = mimeType;
	headers["content-length"] = HttpUtils::numberToString(bodyLen);
	if (statusCode == 405) {
		std::string allowedMethods;
		std::vector<std::string>::const_iterator cit = process->_location.getMethods().begin();
		for (; cit != process->_location.getMethods().end(); ++cit) {
			if (allowedMethods.empty())
				allowedMethods += *cit;
			else
				allowedMethods += " " + *cit;
		}
		headers["allow"] = allowedMethods;
	}
	buildResponse(process, statusCode, headers, body);
}

/* ************************************************************************** */
/*                            response builder utils                          */
/* ************************************************************************** */

std::string ResponseBuilder::createErrorFilePath(ProcessRequest * process, const std::string & errorPage)
{
	if (!process->_request)
		throw HttpErrorException(500, "in RB: Request is NULL.");

	std::string root = selectRoot(process);

	std::string filePath = errorPage;
	HttpUtils::trimFinalSlash(root);
	HttpUtils::trimSlashes(filePath);

	return (root + '/' + filePath);
}

void ResponseBuilder::addFinalHeaders(ProcessRequest * process)
{
	if (process->_location.isCookiesEnabled() && process->_request)
		CookieHandler::handleCookie(process->_httpResponse, *process->_request);
	process->_httpResponse.addHeader("connection", "close");
	if (process->_httpResponse.getHeaders().find("server") == process->_httpResponse.getHeaders().end()) {
		std::ostringstream oss;
		std::vector<std::string>::const_iterator cit = process->_server.getServerNames().begin();
		if (cit != process->_server.getServerNames().end()) {
			oss << *cit;
			++cit;
			for (; cit != process->_server.getServerNames().end(); ++cit) {
				oss << " " << *cit; }}
		process->_httpResponse.addHeader("server", oss.str());
	}
	if (process->_httpResponse.getHeaders().find("content-length") == process->_httpResponse.getHeaders().end())
		process->_httpResponse.addHeader("content-length", HttpUtils::numberToString(process->_httpResponse.getBody().size()));
}

std::string ResponseBuilder::selectErrorPage(ProcessRequest * process, int statusCode)
{
	if (process->_location.hasErrorPage(statusCode)) {
		return (process->_location.getErrorPage(statusCode)); }
	if (process->_server.hasErrorPage(statusCode)) {	
		return (process->_server.getErrorPage(statusCode)); }
	return ("");
}

const std::string & ResponseBuilder::selectRoot(ProcessRequest * process)
{
	return (!process->_location.getRoot().empty() ? process->_location.getRoot() : process->_server.getRoot());
}
