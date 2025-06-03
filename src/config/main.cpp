// #include "../include/config/ConfigParser.hpp"
// #include "../include/config/ServerConfig.hpp"
// #include "../include/config/Location.hpp"

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
//         return 1;
//     }

//     std::string configFile = argv[1];

//     try {
//         ConfigParser parser;
//         parser.parsefile(configFile);

//         const std::vector<ServerConfig>& servers = parser.getServerConfigVector();
//         std::cout << "Nombre de serveurs parsés : " << servers.size() << std::endl;

//         for (size_t i = 0; i < servers.size(); ++i) {
//             const ServerConfig& server = servers[i];
//             std::cout << "Serveur " << (i + 1) << " :" << std::endl;
//             std::cout << "  IP       : " << server.getListen().second << std::endl;
//             std::cout << "  Port     : " << server.getListen().first << std::endl;
//             std::cout << "  Root     : " << server.getRoot() << std::endl;

//             std::cout << "  Server names: ";
//             for (size_t j = 0; j < server.getServerNames().size(); ++j)
//                 std::cout << server.getServerNames()[j] << " ";
//             std::cout << std::endl;

//             std::cout << "  Client max body size: " << server.getClientMaxBodySize() << " octets" << std::endl;

//             std::cout << "  Error pages: " << std::endl;
//             const std::map<int, std::string>& errors = server.getErrorPages();
//             for (std::map<int, std::string>::const_iterator eit = errors.begin(); eit != errors.end(); ++eit)
//                 std::cout << "    " << eit->first << " => " << eit->second << std::endl;

//             std::cout << "  Locations:" << std::endl;
//             const std::map<std::string, Location>& locs = server.getLocations();
//             for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
//                 const Location& loc = it->second;
//                 std::cout << "    Path: " << it->first << std::endl;

//                 std::cout << "      Root        : " << loc.getRoot() << std::endl;
//                 std::cout << "      Index       : " << loc.getIndex() << std::endl;
//                 std::cout << "      Autoindex   : " << (loc.isAutoIndex() ? "on" : "off") << std::endl;
//                 std::cout << "      Methods     : ";
//                 for (size_t k = 0; k < loc.getMethods().size(); ++k)
//                     std::cout << loc.getMethods()[k] << " ";
//                 std::cout << std::endl;
//                 std::cout << "      Upload path : " << loc.getUploadPath() << std::endl;
//                 std::cout << "      CGI ext     : " << loc.getCgiExtension() << std::endl;

//                 if (loc.hasRedirect())
//                     std::cout << "      Redirect    : " << loc.getRedirectCode() << " => " << loc.getRedirectPath() << std::endl;
//             }
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "Erreur : " << e.what() << std::endl;
//         return 1;
//     }

//     return 0;
// }

