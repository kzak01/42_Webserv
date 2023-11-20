#include "libraries/includes.hpp"
#include "libraries/types.hpp"
#include "libraries/classes.hpp"

#define DEFAULT_CONFIG_FILE "configs/test.conf"

//global pointer to WebServer Instance
WebServer *ws_ptr;

void	sigint_handler(int sig)
{
	std::cout << RED_TEXT << "# SIGINT " << sig << " Called #" << RESET_COLOR << std::endl;
	ws_ptr->~WebServer();
	exit(0);
}

int	server_init(const char * path)
{
	try {
		WebServer	Server(path);
		ws_ptr = &Server;
		std::signal(SIGINT, sigint_handler);
		std::cout << "WebServer configured: Starting..." << std::endl;
		Server.start();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return (1);
	}
	return (0);
}

int main(int argc, const char **argv)
{
	int			error_status;
	const char 	*path;


	switch (argc) {
		case 1:
			path = DEFAULT_CONFIG_FILE;
			std::cout << "Running default configuration file <";
			std::cout << path << ">." << std::endl;
			break;
		case 2:
			path = argv[1];
			break;
		default:
			std::cout << RED_TEXT << "Error: Server must be run with only ";
			std::cout << "one <path_to_config_file> argument" << RESET_COLOR << std::endl;
			return (1);
	}

	error_status = server_init(path);
	std::cout << "WebServer shutting down." << std::endl;
	return (error_status);
}

