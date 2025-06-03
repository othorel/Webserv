
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

// #include "../../include/config/ConfigParser.hpp"
#include "../../include/server/Server.hpp"

int	main(void)
{
	// if (argc!= 2)
	// 	return (1);
	try
	{
		// ConfigParser config(argv[1]);
		Server server("debug");
		server.StartEventLoop();
	}
	// catch(ConfigParser::ValidationException & e)
	// {
	// 	std::cerr << e.what() << std::endl;
	// }
	// catch(ConfigParser::ParseException & e)
	// {
	// 	std::cerr << e.what() << std::endl;
	// }
	catch(std::runtime_error & e)
	{
		std::cerr << e.what() << std::endl;
	}	
}
