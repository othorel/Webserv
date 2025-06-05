// #include "../../include/config/ConfigParser.hpp"
// #include "../../include/cgi/CGIHandler.hpp"

// int main(int argc, char** argv) {
//     if (argc < 2) {
//         std::cerr << "Usage: ./test_cgi <config_file>\n";
//         return 1;
//     }

//     try {
//         ConfigParser parser(argv[1]);
//         const std::vector<ServerConfig>& servers = parser.getServerConfigVector();

//         if (servers.empty()) {
//             std::cerr << "No server config found\n";
//             return 1;
//         }

//         // Par exemple, on prend le premier server et sa première location
//         const ServerConfig& server = servers[0];
//         const std::map<std::string, Location>& locations = server.getLocations();

//         if (locations.empty()) {
//             std::cerr << "No locations found in server\n";
//             return 1;
//         }

//         // Récupération de la première location
//         const Location& loc = locations.begin()->second;

//         // Exemples pour construire les paramètres CGI
//         std::string scriptPath = "";  // À définir selon ta config : peut-être un cgi_pass ou root + index ?
//         // Par exemple, concaténation root + index pour le script CGI (à adapter)
//         if (!loc.getCgiExtension().empty())
//             scriptPath = loc.getRoot() + "/test.py";  // ou autre script selon config

//         std::string method = "GET";
//         std::string queryString = "name=olivier";
//         std::string body = "";
//         std::map<std::string, std::string> headers;
//         headers["Content-Type"] = "application/x-www-form-urlencoded";
//         headers["Content-Length"] = "0";

//         CGIHandler cgi(scriptPath, method, queryString, body, headers);
//         std::string output = cgi.execute();

//         std::cout << "CGI output:\n" << output << std::endl;
//     }
//     catch (const std::exception& e) {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }
//     return 0;
// }
