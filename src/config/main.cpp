#include "../include/config/ConfigParser.hpp"
#include "../include/config/ServerConfig.hpp"
#include "../include/config/Location.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::string configFile = argv[1];

    try {
        ConfigParser parser;
        parser.parsefile(configFile);

        const std::vector<ServerConfig>& servers = parser.getServerConfigVector();
        std::cout << "Nombre de serveurs parsés : " << servers.size() << std::endl;

        for (size_t i = 0; i < servers.size(); ++i) {
            const ServerConfig& server = servers[i];
            std::cout << "Serveur " << (i + 1) << " :" << std::endl;
            std::cout << "  IP: " << server.getListen().second << ", Port: " << server.getListen().first << std::endl;
            std::cout << "  Root: " << server.getRoot() << std::endl;

            std::cout << "  Server names: ";
            const std::vector<std::string>& names = server.getServerNames();
            for (size_t j = 0; j < names.size(); ++j) {
                std::cout << names[j] << " ";
            }
            std::cout << std::endl;

            std::cout << "  Locations:" << std::endl;
            const std::map<std::string, Location>& locs = server.getLocations();
            for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
                std::cout << "    Path: " << it->first << std::endl;
                const Location& loc = it->second;

                std::cout << "      Root: " << loc.getRoot() << std::endl;
                std::cout << "      Methods: ";
                const std::vector<std::string>& methods = loc.getMethods();
                for (size_t k = 0; k < methods.size(); ++k)
                    std::cout << methods[k] << " ";
                std::cout << std::endl;

                std::cout << "      Autoindex: " << (loc.isAutoIndex() ? "on" : "off") << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
