#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include "../include/config/ConfigParser.hpp"
#include "../include/config/Location.hpp"
#include "../include/config/ServerConfig.hpp"
#include "../include/server/Server.hpp"
#include "../include/server/PollManager.hpp"
#include "../include/server/Connexion.hpp"
#include "../include/http/HttpErrorException.hpp"
#include "../include/config/ParseException.hpp"

volatile __sig_atomic_t g_stop = 0;

void	printBanner()
{
	std::cout <<
	"             _    _ ___________  _____ ___________ _   _  \n"
	"            | |  | |  ___| ___ \\/  ___|  ___| ___ \\ | | | \n"
	"            | |  | | |__ | |_/ /\\ `--.| |__ | |_/ / | | | \n"
	"            | |/\\| |  __|| ___ \\ `--. \\  __||    /| | | | \n"
	"            \\  /\\  / |___| |_/ //\\__/ / |___| |\\ \\\\ \\_/ / \n"
	"             \\/  \\/\\____/\\____/ \\____/\\____/\\_| \\_|\\___/  \n"
	"\n"
	"\033[36m"
    "      _________________________________________________________\n"
    "     |                                                         |\n"
    "     |   .-------------------------------------------------.   |\n"
    "     |   |  \033[32m.-----------------.   .---------------------.\033[36m  |   |\n"
    "     |   |  \033[32m|   * WEBSERV *   |   |     SERVER READY    |\033[36m  |   |\n"
    "     |   |  \033[32m'-----------------'   '---------------------'\033[36m  |   |\n"
    "     |   |  \033[35m .----------------.\033[36m                            |   |\n"
    "     |   |  \033[35m|   ____________   |\033[36m     [:::]      [:::]      |   |\n"
    "     |   |  \033[35m|  |  ________  |  |\033[36m     [:::]      [:::]      |   |\n"
    "     |   |  \033[35m|  | |        | |  |\033[36m     [:::]      [:::]      |   |\n"
    "     |   |  \033[35m|  | |        | |  |\033[36m     [:::]      [:::]      |   |\n"
    "     |   |  \033[35m|  | |________| |  |\033[31m     _________________ \033[36m    |   |\n"
    "     |   |  \033[35m|  |____________|  |\033[31m    |#################|\033[36m    |   |\n"
    "     |   |  \033[35m|                  |\033[31m    |#################|\033[36m    |   |\n"
    "     |   |  \033[35m'------------------'\033[31m    '-----------------'\033[36m    |   |\n"
    "     |   '-------------------------------------------------'   |\n"
    "     |                                                         |\n"
    "     |_________________________________________________________|\n"
	"                              |       |                         \n"
	"                              |       |                         \n"
	"\033[33m"
    "        .---------------------------------------------------.\n"
    "       / \033[34m __  __  __  __  __  __  __  __  __  __  __  __  __ \033[33m\\\n"
    "      /  \033[34m|__||__||__||__||__||__||__||__||__||__||__||__||__|\033[33m \\\n"
    "     /   \033[34m|__||__||__||__||__||__||__||__||__||__||__||__||__|\033[33m  \\\n"
    "    /___________________________________________________________\\\n"
    "   |_____________________________________________________________|\n"
    "\n"
	"\033[0m"
    << std::endl;
}

void handle_sigint(int signal)
{
	(void)signal;
	g_stop = 1;
	std::cout << "\n\033[1;31m[Signal] Ctrl+C received, server shutdown requested...\033[0m";
}

int	main(int argc, char **argv)
{
	signal(SIGINT, handle_sigint);

	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
 		return (1);
	}
	std::string	configFile;
	if (argc == 1)
		configFile = "./config/default.conf";
	else
		configFile = argv[1];
	try {
		ConfigParser parser(configFile);
		Server server(parser);
		printBanner();
		server.announce();
		server.StartEventLoop();
	}
	catch (const HttpErrorException& e) {
		std::cerr << "[HttpError] " <<  e.what() << " (status " << e.getStatusCode()  << ")" << std::endl;
		return (1);
	}
	catch (const ParseException& e) {
		std::cerr << "[ConfigError] " << e.what() << std::endl;
		return (1);
	}
	catch (const std::exception& e) {
		if (g_stop == 0)
			std::cerr << "[Fatal] " << e.what() << std::endl;
		return (1);
	}
}
