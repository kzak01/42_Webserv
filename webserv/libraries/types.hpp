#ifndef TYPES_HPP
# define TYPES_HPP
# include "includes.hpp"

class BadInputException : public std::exception {
public:
	const char* what() const throw() {
		return "Error: Bad Input";
	}
};

// SERVER STRUCTURES

typedef struct s_socket
{
	sockaddr_in	sockaddr;
	int			sock_fd;
	socklen_t	sockaddr_len;
}	t_socket;

typedef struct s_location
{
	std::string proxy_pass;
}	t_location;

typedef	struct s_serv
{
	sockaddr_in				cli_socket;
	int						cli_fd;

	epoll_event				ep_event;

	size_t					port;
	std::string 			name;
	std::vector<t_location>	locations;
}	t_serv;

// typedef struct s_epoll
// {
// 	int						epoll_fd;
// 	int						*server_sockets;
// }	t_epoll;


#endif