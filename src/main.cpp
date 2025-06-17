#include <signal.h>
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
#include "../include/config/ParseException.hpp"

volatile sig_atomic_t g_stop = 0;

void handle_sigint(int signal) {
	(void)signal;
	g_stop = 1;
	std::cout << "\n[Signal] Ctrl+C received, server shutdown requested...";
}

int	main(int argc, char **argv) {
	signal(SIGINT, handle_sigint);
	if (argc!= 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
 		return (1);
	}

	std::string	configFile = argv[1];

	try
	{
		ConfigParser parser(configFile);
		parser.debug();
		Server server(parser);
		server.announce();
		server.StartEventLoop();
	}
	catch (const HttpErrorException& e)
	{
		std::cerr << "[HttpError] " <<  e.what() << " (status " << e.getStatusCode()  << ")" << std::endl;
		return (1);
	}
	catch (const ParseException& e)
	{
		std::cerr << "[ConfigError] " << e.what() << std::endl;
		return (1);
	}
	catch (const std::exception& e)
	{
		std::cerr << "[Fatal] " << e.what() << std::endl;
		return (1);
	}
}

