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

#ifdef __APPLE__
	volatile sig_atomic_t g_stop = 0;
#else
	volatile __sig_atomic_t g_stop = 0;
#endif

void	printBanner()
{
	std::cout <<
	"\n"
	"\033[36m"
    "      _________________________________________________________\n"
    "     |                                                         |\n"
    "     |   .-------------------------------------------------.   |\n"
    "     |   |  \033[38;2;214;41;118m _    _ ___________  _____ ___________ _   _\033[36m   |   |\n"
    "     |   |  \033[38;2;214;41;118m| |  | |  ___| ___ \\/  ___|  ___| ___ \\ | | |\033[36m  |   |\n"
    "     |   |  \033[38;2;224;68;89m| |  | | |__ | |_/ /\\ `--.| |__ | |_/ / | | |\033[36m  |   |\n"
    "     |   |  \033[38;2;234;95;59m| |/\\| |  __|| ___ \\ `--. \\  __||    /| | | |\033[36m  |   |\n"
    "     |   |  \033[38;2;245;122;30m\\  /\\  / |___| |_/ //\\__/ / |___| |\\ \\\\ \\_/ / \033[36m |   |\n"
    "     |   |  \033[38;2;255;149;0m \\/  \\/\\____/\\____/ \\____/\\____/\\_| \\_|\\___/\033[36m   |   |\n"
    "     |   |  \033[35m.------------------.\033[36m                           |   |\n"
    "     |   |  \033[35m|  .------------.  |\033[31m     [:::]      [:::]  \033[36m    |   |\n"
    "     |   |  \033[35m|  |            |  |\033[31m     [:::]      [:::]  \033[36m    |   |\n"
    "     |   |  \033[35m|  |            |  |\033[31m     [:::]      [:::]  \033[36m    |   |\n"
    "     |   |  \033[35m|  '------------'  |\033[31m     [:::]      [:::]  \033[36m    |   |\n"
    "     |   |  \033[35m'------------------'\033[31m                       \033[36m    |   |\n"
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
