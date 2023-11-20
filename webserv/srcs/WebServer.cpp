# include "WebServer.hpp"

WebServer::WebServer()
{
	std::cout << "Warning, empty WebServer instance" << std::endl;
}

WebServer::WebServer(const char* config_path)
{
	std::string	input_path(config_path);
	std::string	sanitized_path;

	SANITIZE_STRING(input_path, sanitized_path);

	std::cout << "Creating an Event Poll" << std::endl;

	std::cout << "Parsing configuration file <" << sanitized_path << "> ..." << std::endl;

	if (this->_parse_config_create_servers(sanitized_path))
		throw ParsingException();
	if (this->_create_listening_sockets())
		throw SocketException();
	if (this->_init_epoll())
		throw EpollInitException();
	if (this->_create_servers_epollevent())
		throw EpollInitException();
}

int WebServer::_parse_config_create_servers(const std::string& path)
{
	Config config(path, this->_servers);
	
	if (config.conf_file_ok == false)
		return 1;
	//test_sServ_struct();
	return (0);
}

//prints all the elements of _servers vector
void WebServer::test_sServ_struct() {

	std::cout << "--------------test_sServ_struct------------------" << std::endl;
	int count = 1;

	std::vector<sServ>::const_iterator it;

	for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
		std::cout << std::endl << "--------------- SERVER " << count << " ---------------" << std::endl << std::endl;
		std::cout << *it << std::endl;
		count++;
	}
	std::cout << std::endl;
}

int WebServer::_create_listening_sockets()
{
	std::cout << std::endl << ">>>> Configuring Sockets, number of Servers: " << this->_servers.size() << std::endl;
	for (size_t x = 0; x < this->_servers.size(); x++)
	{
		std::cout << std::endl << "--------------- SERVER " << x + 1 << " ---------------" << std::endl << std::endl;
		std::cout << ">> SERVER " << x + 1 << " BEING CREATED " << std::endl;
		this->_servers[x].print();
		std::cout << "SOCKET CREATION " << this->_servers[x].ip_address << ":" << this->_servers[x].port << std::endl;
		this->_servers[x]._listening_socket.sockaddr.sin_family = AF_INET;
		this->_servers[x]._listening_socket.sockaddr.sin_port = htons(atoi(this->_servers[x].port.c_str()));
		this->_servers[x]._listening_socket.sockaddr.sin_addr.s_addr = inet_addr(this->_servers[x].ip_address.c_str());
		this->_servers[x]._listening_socket.sockaddr_len = sizeof(this->_servers[x]._listening_socket.sockaddr);
		this->_servers[x]._listening_socket.sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		int reuse = 1;
		if (setsockopt(this->_servers[x]._listening_socket.sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
		        close(this->_servers[x]._listening_socket.sock_fd);
		        return 1;
		    }

		this->_bind_socket(this->_servers[x]._listening_socket);
		if (listen(this->_servers[x]._listening_socket.sock_fd, SOMAXCONN) == -1)
		{
			std::cout << RED_TEXT << "Error in setup socket listen" << RESET_COLOR << std::endl;
			return 1;
		}
	}
	std::cout << std::endl << "----------------------------------------" << std::endl;
	std::cout << ">>> END OF SERVER SOCKETS CREATION" << std::endl << std::endl;
	return (0);
}

int WebServer::_bind_socket(sSocket& ref)
{
	if (bind(ref.sock_fd, (sockaddr *)&ref.sockaddr, ref.sockaddr_len) == -1)
		{
			std::cout << RED_TEXT << "Binding Error" << RESET_COLOR << std::endl;
		//	exit (1);
		} 
	return 0;
}

/**
 * @brief This method calls the epoll create sys call. The event poll
 * needs a file descriptor, and epoll_create provides such. 
 * epoll_create1 is an updated sys call with additional features.
 *
 * @param ref 
 */
int WebServer::_init_epoll()
{
	this->epoll_fd = epoll_create1(EPOLL_CLOEXEC);	//CLOEXEC argument is made to prevent fd inheritance by forked processes.
	if (this->epoll_fd < 0) //! not sure which fd is assigned on error
		return (1);
	std::cout << "epoll created:" << this->epoll_fd << std::endl;
	return 0;
}


int WebServer::_create_servers_epollevent()
{
	for (size_t x = 0; x < this->_servers.size(); x++)
	{
		this->_servers[x]._event.events = EPOLLIN;
		this->_servers[x]._event.data.ptr = &this->_servers[x];

		this->_servers[x].socket_type = SERVER_SOCKET;		// IT'S GOING TO BE A SERVER SOCKET!
	}
	std::cout << "Server_socket-S events created." << std::endl;
	return (0);	
}

WebServer::~WebServer()
{
	std::list<sConnection>::iterator cli_it = _clients.begin();
	while (cli_it != _clients.end())
	{
		close(cli_it->connection_socket.sock_fd);
		if (cli_it->request)
			delete(cli_it->request);
		if (cli_it->response)
			delete(cli_it->response);
		++cli_it;
	}

	std::vector<sServ>::iterator serv_it = _servers.begin();
	while (serv_it != _servers.end())
	{
		close(serv_it->_listening_socket.sock_fd);
		++serv_it;
	}
}

int WebServer::start()
{
	std::cout << "CALLED START!!!!!" << std::endl;
	if (this->_epoll_load_serversocket_events())
		throw GenericEpollException();
	if (this->_event_loop())
		throw EventException();
	return(0);
}

int WebServer::_epoll_load_serversocket_events()
{
	for (size_t x = 0; x < this->_servers.size(); x++)
	{
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->_servers[x]._listening_socket.sock_fd, &this->_servers[x]._event))
			return 1;
	}
	std::cout << "events added to epoll pool" << std::endl;
	return 0;
}

int WebServer::_epoll_load_event(epoll_event &event)
{
	sConnection *ptr = static_cast<sConnection *>(event.data.ptr);

	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, ptr->connection_socket.sock_fd, &event))
		return 1;
	return 0;
}

int WebServer::_event_loop()
{
	int	epoll_scan = 0;
	epoll_event	events[EVENT_QUEUE];
	do
	{
		epoll_scan = epoll_wait(this->epoll_fd, events, EVENT_QUEUE, EPOLL_TIMEOUT);
		if (epoll_scan > 0) {
			try {this->_handle_events(events, epoll_scan);}
			catch(const std::exception& e) {
				std::cout << RED_TEXT << e.what() << RESET_COLOR << '\n';
				return 1; //!
			}
		}
	} while (epoll_scan >= 0);
	if (epoll_scan < 0) {
		std::cout << RED_TEXT << "epoll_scan < 0" << RESET_COLOR << std::endl;
		return 1;
	}
	return 0;
}

int WebServer::stop()
{
	return(0);
}

int WebServer::_handle_events(epoll_event (&events)[EVENT_QUEUE], int &epoll_scan)
{
	for (int i = 0; i < epoll_scan; ++i)
	{
		sBase	*ptr = static_cast<sBase *>(events[i].data.ptr);

		if (ptr->socket_type == SERVER_SOCKET)
		{
			std::cout << "Is a new connection." << std::endl;
			if (this->_accept_connection(events[i]))
			{
				std::cout << RED_TEXT << "Error in Accepting connection." << RESET_COLOR << std::endl;
			}	//! in caso di errore gestione errore
		}
		else if (((events[i].events & EPOLLIN) || (events[i].events & EPOLLOUT))
			&& this->_handle_connection(events[i]))
			{
				std::cout << RED_TEXT << "Error in Handling Connection" << RESET_COLOR << std::endl;
				return 1;			//! HERE I THINK ABORTED OR CORRUPTED CONNECTIONS SHALL BE HANDLED
			}
	}
	return(0);
}

/**
 * @brief Creates a new Client struct to add to the _connections list. It initializes it's socket structures
 * relatively to the server configurations (the epoll_event linked to the server_socket points to server data).
 * It then accepts a new connection (accept call) and stores this socket fd in the sSocket structure inside the
 * sConnection structure. New socket returnet from the accept call is then binded (bind() call). An event for this
 * connection is then created and passed to epoll.
 * 
 * @param event 
 * @return int 
 */
int WebServer::_accept_connection(epoll_event &event)
{
	sConnection	newCli;
	epoll_event	newEvent;
	sServ *ptr = static_cast<sServ *>(event.data.ptr);

	//SOCKET HANDLING
//	newCli.status = true;
	newCli.id = clock();
	if ((newCli.connection_socket.sock_fd = accept(ptr->_listening_socket.sock_fd, (sockaddr *)&ptr->_listening_socket.sockaddr, &ptr->_listening_socket.sockaddr_len)) == -1)
	{
		std::cout << "On socket: " << ptr->_listening_socket.sock_fd;
		std::cout << " No connection can be estabilished" << std::endl;
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return 0;
		else
			return 1;
	}
	_set_sock_timeout(newCli.connection_socket, 10, 0);

	// SET NON BLOCKING
	int flags = fcntl(newCli.connection_socket.sock_fd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	fcntl(newCli.connection_socket.sock_fd, F_SETFL, flags);

	std::cout << "CONNECTION ESTABILISHED" << std::endl;
	_set_socket((*ptr), newCli.connection_socket);

	//EVENT HANDLING
	newEvent.events = EPOLLIN | EPOLLOUT;
	newCli.socket_type = CLIENT_SOCKET;

	// LINK SERVER POINTER INTO CLIENT
	newCli.server = ptr;
	
	//ADD CONNECTION TO LIST AND LOAD IT IN EPOLL
	this->_clients.push_back(newCli);	// a newCli is created by copy when added to list.
	newEvent.data.ptr = &this->_clients.back(); // Event should store the ref to the listed struct.
	if (this->_epoll_load_event(newEvent))
	{
		std::cout << RED_TEXT << "Epoll Loading Error" << RESET_COLOR << std::endl;
		return 1;
	}
	return 0;
}

int WebServer::_set_socket(sServ &serv, sSocket &sock)
{
	sock.sockaddr.sin_family = AF_INET;
	sock.sockaddr.sin_port = htons(atoi(serv.port.c_str()));
	sock.sockaddr.sin_addr.s_addr = inet_addr(serv.ip_address.c_str());
	sock.sockaddr_len = sizeof(sock.sockaddr);

	return 0;
}

/**
 * @brief Each Event as returnet from epoll_wait is handled
 * individually. A Class for each task is instanciated, except
 * for send method which handles writing an HTTP RESPONSE to
 * the socket. Each Class evaluates the situation and operates
 * only if required. The flow is inteded as follows:
 * - 1 : Reading from Socket. If Socket is not empty, a sRequest 
 * 		and a sResponse are initializated. sRequest is filled
 * 		accordingly.
 * - 2 : if sRequest is complete, it gets parsed.
 * - 3 : if sRequest is complete and a body is expected, a body
 * 		is readen until a complete body_temp_file is written.
 * - 4 : when sRequest and the optional body_temp_file are complete
 * 		, sRequest get handled by two Handling functions, which
 * 		perform the HTTP REQUEST and fill out the sResponse accordingly.
 * 		These two Classes are one for Static file handling, and one
 * 		for handling files that require a CGI.
 * - 5 : if sResponse is complete, the WebServer will send a Response
 * 		through the socket.
 * - 6 : When the sResponse has been completely sent via Socket,
 * 		connection is then closed, socket gets closed, memory is freed 
 * 		up accordingly.
 * 
 * @param event 
 * @return int 
 */
int WebServer::_handle_connection(epoll_event &event)
{
	sConnection& ref = *reinterpret_cast<sConnection*>(event.data.ptr);


	if (ref.takeConnectionTime == 0) {
		ref.takeConnectionTime++;
		ref.startConnectionTime = std::time(NULL);
	}
	std::time_t currentTime = std::time(NULL);
	double elapsedTime = std::difftime(currentTime, ref.startConnectionTime);
	if (elapsedTime >= 10) {
		_close_connection(event);
		return (0);
	}

	HttpRequest			request(ref);
	if (ref.errorClose == true) {
		_close_connection(event);
		return (0);
	}
	if (ref.request && ref.request->complete == true) {
		ParseRequest	parser(ref);
		BodyRead		body(ref);
		if (ref.errorClose == true) {
			_close_connection(event);
			return 0;
		}
		CGI_Handler		cgihandler(ref);
		S_Handler		shandler(ref);

		try {
			_send_response(ref);
		}
		catch (std::exception &error){
			debug_printing(E_ERROR, "Send Response error -> closing connection");
			_close_connection(event);
			return (0);
		}
		if (ref.request && ref.request->answered == true)
			_close_connection(event);
	}
	return 0;
}

/**
 * @brief remove event from pool, close socket, clear client, remove client from list
 * 
 * @param event 
 * @return int 
 */
int WebServer::_close_connection(epoll_event &event)
{
	sConnection& ref = *reinterpret_cast<sConnection*>(event.data.ptr);
	std::cout << GREEN_TEXT << "-Close Connection" << RESET_COLOR << std::endl;

	int keepAlive = 0;
	if (ref.request && ref.request->answered == true)
	{
		if (ref.request->headers["Connection"] == "keep-alive")
			keepAlive = 1;
		delete (ref.request);
		ref.request = NULL;
	}
	if (ref.response)
	{
		delete (ref.response);
		ref.response = NULL;
	}

	if (keepAlive == 1)
		return 0;

	std::cout << RED_TEXT << "close/timeout\n" << RESET_COLOR << std::endl;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ref.connection_socket.sock_fd, &event) == -1)
		return 1;
	if (close(ref.connection_socket.sock_fd) == -1)
		return 1;

	this->_clients.remove(ref);
	return 0;
}

int WebServer::_set_sock_timeout(sSocket &sock, size_t sec, size_t usec)
{
	struct	timeval	timeout;

	timeout.tv_sec = sec;
	timeout.tv_usec = usec;

	setsockopt(sock.sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	return 0;
}

void WebServer::test_sServ()
{
	std::cout << "########### CONFIG TEST ##########" << std::endl;
	for (size_t x = 0; x < _servers.size() ; ++x)
		_servers[x].print();
	std::cout << "############### END ##############" << std::endl;
}

// int	setsockopt(int socket_fd)
// {
// 	int error_code = 0;
// 	unsigned int error_code_size = sizeof(error_code);
// 	getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
// 	return (error_code);
// }

void closeFD(int fd)
{
	if (fd != -1)
		close(fd);
}


void	send_stuff(int fd, char *body, long int size)
{
	// if (0)
	// {
	// 	debug_printing(E_ERROR, "ERrOr writing into socket..");
	// 	throw SendException();
	// }

	switch (send(fd, body, size, 0)){
		case -1:
			debug_printing(E_ERROR, "ERrOr writing into socket..");
			delete[] body;
			throw SendException();

		case 0:
			debug_printing(E_WARNING, "Sent 0 bytes");
			delete[] body;
			throw SendException();
		default :
			delete[] body;
	}
}

std::string	generate_random_cookie()
{
	std::string alphabet = "0123456789qwertyuioplkjhgfdsazxcvbnm";
	int	len = alphabet.size();
	std::string cookie = "";

	for (int i = 0; i < 20; i++)
		cookie.push_back( alphabet[std::rand() % len] );
	return cookie;
}

void	create_cookie(sConnection &conn)
{
	if (!(conn.request->headers.find("Cookie") == conn.request->headers.end()))
	{
		if (!(conn.request->headers["Cookie"].find("webserv_cookie") == std::string::npos))
			return ;
	}
	debug_printing(E_DEBUG, "Generating cookie");

	std::string header = ""; //= &conn.response->header;
	header += "\r\nSet-Cookie: webserv_cookie=";
	header += generate_random_cookie();
	header += "\r\n\r\n";

	conn.response->header.replace(conn.response->header.size() - 4, 4, header);
}

typedef unsigned long int t_uli;

int WebServer::_send_response(sConnection &connection)
{
	static int i = 0;
	if (i == 0) {
		i++;
		return 0;
	}
	i--;
	sResponse	*r = connection.response;
	if (r && r->complete == true)
	{
		std::cout << std::endl << GREEN_TEXT << "-RESPONSE-" << RESET_COLOR << std::endl;
		std::cout << YELLOW_TEXT << connection.response->header << RESET_COLOR << std::endl;
		
	//	t_uli		body_sz = 0;

		create_cookie(connection);

		int			sock_fd		= connection.connection_socket.sock_fd;
		t_uli		header_sz	= connection.response->header.length();
		t_uli		body_sz		= connection.response->file_stat.st_size;



		if (header_sz + body_sz + 4 > MAX_RESPONSE_SIZE && connection.getStatusCode() != 500) {
			connection.setStatusCode(500);
			r->complete = false;
			return 0;
		}

		char		*response = new char[header_sz + body_sz + 4];
		memset(response, 0, header_sz + body_sz + 4);
		for (t_uli i = 0; i < header_sz; i++)
			response[i] = connection.response->header[i];

		int	body_fd = open(r->file_path.c_str(), O_RDONLY);
		if (body_fd > 0 && body_sz > 0)
		{
			char			body[body_sz];
			memset(body, 0, body_sz);

			int	siz = read(body_fd, body, body_sz);
			switch (siz){
				case -1:
					debug_printing(E_ERROR, "ERrOr reading file..");
					break ;
				case 0:
					debug_printing(E_WARNING, "Body size 0");
					break ;
				default:
					debug_printing(E_DEBUG, "Body read -> " + numToString(siz) + " Byte");
			}
			closeFD(body_fd);

			std::string body_print(body);
			if (body_print.size() > 0)
				std::cout << YELLOW_TEXT << body_print.substr(0, body_sz) << RESET_COLOR << std::endl;
			for (t_uli i = 0; i < (t_uli) body_sz; i++)
				response[header_sz + i] = body[i];

			// response[header_sz + body_sz + 1] = '\r';
			// response[header_sz + body_sz + 2] = '\n';
			// response[header_sz + body_sz + 3] = '\r';
			// response[header_sz + body_sz + 4] = '\n';

			response[header_sz + body_sz] = '\r';
			response[header_sz + body_sz + 1] = '\n';
			response[header_sz + body_sz + 2] = '\r';
			response[header_sz + body_sz + 3] = '\n';
		}
		send_stuff(sock_fd, response, header_sz + body_sz + 4);
		connection.request->answered = true;
	}
	return 0;
}
