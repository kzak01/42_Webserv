#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP
#include "../libraries/types.hpp"
#include "../libraries/includes.hpp"
#include "../libraries/classes.hpp"

class WebServer
{
	private:
		std::vector<sServ> _servers;
		std::list<sConnection> _clients;
		int epoll_fd;
		
		void test_sServ_struct();

		// INITIALIZATION
		int		_parse_config_create_servers(const std::string &path);
		int		_create_listening_sockets();
		int		_init_epoll();
		int		_create_servers_epollevent();
		int		_bind_socket(sSocket &ref);
		// EXECUTION
		int		_epoll_load_serversocket_events();
		int		_epoll_load_event(epoll_event &event);
		int		_event_loop();
		int		_accept_connection(epoll_event &event);
		int		_handle_connection(epoll_event &event);
		int		_set_socket(sServ &serv, sSocket &sock);
		int		_close_connection(epoll_event &event);
		int		_set_sock_timeout(sSocket &sock, size_t sec, size_t usec);
		int 	_accept_socket(sConnection& connection);
		int		_send_response(sConnection &ref);
		//	EPOLL METHODS
		int 	_handle_events(epoll_event (&events)[EVENT_QUEUE], int &epoll_scan);
		int 	_EPOLLIN_event(epoll_event &event);

	public:
		WebServer();
		WebServer(const char *config_path);
		WebServer(const WebServer &instance);
		WebServer &operator=(const WebServer &instance);
		~WebServer();

		int		start();
		int		stop();
		void	test_sServ();

	// CLASS EXCEPTIONS
	class ParsingException : public std::exception
	{
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR 1: PARSING FILE ERROR");
		}
	};
	class EpollInitException : public std::exception
	{
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR 2: SERVER EVENTS INITIALIZATION ERROR");
		}
	};
	class SocketException : public std::exception
	{
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR 3: SOCKET CREATION ERROR");
		}
	};
	class GenericEpollException : public std::exception
	{
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR 4: EPOLL_CTL ERROR");
		}
	};
	class EventException : public std::exception
	{
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR 4: EVENT ERROR");
		}
	};
};
	
class SendException : public std::exception
{
public:
	virtual const char *what() const throw()
	{
		return ("ERROR 5: SEND ERROR, but for real please");
	}
};

#endif
