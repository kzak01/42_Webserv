#ifndef INCLUDES_HPP
# define INCLUDES_HPP

# include <iostream>
# include <iomanip>
# include <cstddef>
# include <ctime>
# include <cctype>
# include <limits>
# include <fstream>
# include <sstream>
# include <string>
# include <cstring>
# include <algorithm>

# include <fcntl.h>
# include <cstdlib>
# include <exception>
# include <netinet/in.h>
# include <signal.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <dirent.h>

// NETWORKING
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <unistd.h>	// close, read, write

// CONTAINERS
# include <vector>
# include <map>
# include <list>
# include <deque>
# include <stack>



// UTILITIES
# include "sanitize.hpp"
# include "../srcs/utils.hpp"
# include <csignal>


# include "sys/stat.h"

// UTILITY DEFINES
# define S_IP			"127.0.0.1"
# define S_PORT			8080
# define BUFFER_SIZE	102400
# define EPOLL_TIMEOUT	2000
# define EVENT_QUEUE	1024
# define SERVER_SOCKET  1
# define CLIENT_SOCKET  2
# define PATH_MAX		4096
# define EMPTY			1000

# define PORT_MAX_LENGTH 5
# define ONE_KILOBYTE 1024
# define ONE_MEGABYTE 1048576

# define MAX_REQUEST_SIZE 4000000
# define MAX_RESPONSE_SIZE 4000000
# define MAX_BODY_SIZE 4194305  //4MB + 1 Byte
# define MAX_HEADER_SIZE 4096   //4KB


// Colours
// Sequenze di escape ANSI per colori del testo
# define RESET_COLOR "\033[0m"
# define BLACK_TEXT "\033[0;30m"
# define RED_TEXT "\033[0;31m"
# define GREEN_TEXT "\033[0;32m"
# define YELLOW_TEXT "\033[0;33m"
# define BLUE_TEXT "\033[0;34m"
# define MAGENTA_TEXT "\033[0;35m"
# define CYAN_TEXT "\033[0;36m"
# define WHITE_TEXT "\033[0;37m"

// Sequenze di escape ANSI per colori del testo in grassetto
# define BOLD_BLACK_TEXT "\033[1;30m"
# define BOLD_RED_TEXT "\033[1;31m"
# define BOLD_GREEN_TEXT "\033[1;32m"
# define BOLD_YELLOW_TEXT "\033[1;33m"
# define BOLD_BLUE_TEXT "\033[1;34m"
# define BOLD_MAGENTA_TEXT "\033[1;35m"
# define BOLD_CYAN_TEXT "\033[1;36m"
# define BOLD_WHITE_TEXT "\033[1;37m"

# define DEBUG_LEVEL 2
enum {E_DEBUG, E_INFO, E_WARNING, E_ERROR};

template <typename T>
void	debug_printing(int debug_code, T message){
	if (debug_code < DEBUG_LEVEL)
		return ;
	
	std::string color = WHITE_TEXT;
	std::string debug_name = "NONE  ";

	switch (debug_code){
		case E_DEBUG:
			color = CYAN_TEXT;
			debug_name = "DEBUG ";
			break ;
		case E_INFO:
			color = GREEN_TEXT;
			debug_name = "INFO  ";
			break;
		case E_WARNING:
			color = YELLOW_TEXT;
			debug_name = "WARN  ";
			break;
		case E_ERROR:
			color = RED_TEXT;
			debug_name = "ERROR ";
			break;
	}

	std::cout << "[" << color << debug_name << RESET_COLOR << "]: ";
	std::cout << message;
	std::cout << std::endl; }

// STRUCTURES
# include "../libraries/structures.hpp"

#endif
