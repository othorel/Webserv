#include <cstdlib>
#include <ctime>
#include <string>
#include "../../include/process/CookieHandler.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpUtils.hpp"
#include "../../include/config/Location.hpp"

void CookieHandler::handleCookie(
	HttpResponse & response, const HttpRequest & request)
{
	if (request.hasHeader("cookie") && !request.getHeaderValue("cookie").empty())
		return ;

	std::string randomString  = generateRandomString(10);

	std::string cookie =
		"session_id=" + randomString + "; Path=/; HttpOnly";
	
	response.addHeader("Set-Cookie", cookie);
}

std::string CookieHandler::generateRandomString(size_t size)
{
	static bool seeded = false;
	if (!seeded) {
		std::srand(std::time(NULL));
		seeded = true;
	}
	std::string result;
	for (size_t i = 0; i < size; ++i) {
		char letter = 'a' + std::rand() % 26;
		result += letter;
	}
	return (result);
}
