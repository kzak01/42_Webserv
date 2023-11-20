#include "Client.hpp"

int Client::_create_socket()
{
	std::cout << "SOCKET CREATION " << std::endl;
	this->_server.sockaddr.sin_family = AF_INET;
	this->_server.sockaddr.sin_port = htons(8080);
	this->_server.sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	this->_server.sockaddr_len = sizeof(this->_server.sockaddr);
	this->_server.sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	return 0;
}

Client::Client()
{
	_create_socket();
}

Client::~Client()
{
		
}

int Client::start()
{
	if (connect(_server.sock_fd, (struct sockaddr*)&_server.sockaddr, sizeof(_server.sockaddr)) == -1) {
		std::cout << RED_TEXT << "Error connecting to the server" << RESET_COLOR << std::endl;
		exit(EXIT_FAILURE);
	}

	// std::cout << "Type message to send" << std::endl;
	// while (true) {
	//	 std::string	 input;
	//	 std::getline(std::cin, input);
	//	 int bytesSent = send(_server.sock_fd, input.c_str(), input.size(), 0);
	//	 if (bytesSent == -1) {
	//			 perror("Error sending data to server");
	//			 close(_server.sock_fd);
	//			 exit(EXIT_FAILURE);
	// }
	// }

	// STATIC CONTENT
	std::string	input("GET /test.png HTTP/1.1\nHost: 127.0.0.1:8080\r\n\r\n");
	int bytesSent = send(_server.sock_fd, input.c_str(), input.size(), 0);
	if (bytesSent == -1) {
		perror("Error sending data to server");
		close(_server.sock_fd);
		exit(EXIT_FAILURE);
	}

	// char	answer[1000];

	char buffer[2048];
	ssize_t bytesRead;
	do {
		std::cout << "Peeking" << std::endl;
		bytesRead = recv(_server.sock_fd, buffer, 2048, MSG_PEEK);
		if (bytesRead == -1 || bytesRead == 0)
		{
			std::cout << "Peeked empty socket" << std::endl;
			//return;
		}
	} while (bytesRead == -1 || bytesRead == 0);
	// read(_server.sock_fd, answer, 1000);

	std::cout << buffer << std::endl;
	return 0;
}
