// #include "../../include/cgi/CGIHandler.hpp"

// int main() {
// 	try {
// 		std::map<std::string, std::string> headers;
// 		headers["Content-Type"] = "application/x-www-form-urlencoded";
// 		headers["Content-Length"] = "0";
// 		std::string body = "name=test&age=42";

// 		CGIHandler handler(
// 			"/home/olthorel/github/Webserv/www/cgi-bin/test_post.py", // chemin vers le script CGI
// 			"POST",                // méthode
// 			"",                   // query string
// 			body,                   // body
// 			headers               // headers
// 		);
// 		std::string output = handler.execute();
// 		std::cout << "CGI Output:\n" << output << std::endl;
// 	}
// 	catch (const CGIHandler::CGIException &e) {
// 		std::cerr << "CGIException: " << e.what() << std::endl;
// 	}
// 	catch (const std::exception &e) {
// 		std::cerr << "Exception: " << e.what() << std::endl;
// 	}

// 	return 0;
// }
