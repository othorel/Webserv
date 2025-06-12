
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include "../include/config/ConfigParser.hpp"
#include "../include/config/Location.hpp"
#include "../include/config/ServerConfig.hpp"
#include "../include/server/Server.hpp"
#include "../include/server/PollManager.hpp"
#include "../include/server/Connexion.hpp"
#include "../include/http/HttpErrorException.hpp"

int	main(int argc, char **argv)
{
	if (argc!= 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
 		return (1);
	}

	std::string	configFile = argv[1];

	try
	{
		ConfigParser parser(configFile);
		
		// const std::vector<ServerConfig>& servers = parser.getServerConfigVector();
		// std::cout << "Nombre de serveurs parsés : " << servers.size() << std::endl;

		// for (size_t i = 0; i < servers.size(); ++i)
		// {
		// 	servers[i].printServerConfig(i);
		// }

		// std::cout << "\nValidation réussie : la configuration est correcte." << std::endl;

		Server server(parser);

		std::cout << "\nCréation des sockets" << std::endl;

		server.StartEventLoop();
	}
	catch (const HttpErrorException& e)
	{
		std::cerr << e.what() << " " << e.getStatusCode() << std::endl;
		return (1);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}	
}


// pour tester ecrire sur un deuxieme terminal : curl http://127.0.0.1:8080/ (ou un des couples IP port correspondant au parsing)
