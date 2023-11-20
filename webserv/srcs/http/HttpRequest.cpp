#include "HttpRequest.hpp"

//peek max_size in buffer
//is there a CRCR in buffer?
// se si sizetoread = size da 0 a CRCR
// se no return.

//less than max size - no CRCR
	// max size - no CRCR
// less than max size - CRCR
	// max size - CRCR
//more than max size - no CRCR
//


HttpRequest::HttpRequest(sConnection& client) {
	try {
		// std::cout << "a" << std::endl;
		if (client.request)
		// {
		// 	std::cout << "i" << std::endl;
			return ;
		// }
		if (client.hpeaked == false)
		{
		//	int	size;
			char buffer[MAX_HEADER_SIZE];
			memset(buffer, 0, MAX_HEADER_SIZE);

			client.peaksize = recv(client.connection_socket.sock_fd, buffer, MAX_HEADER_SIZE, MSG_PEEK);
			if (client.peaksize == -1)
			{
				client.errorClose = true;
				//std::cout << "peeked empty socket" << std::endl;
				return; // std::cout << "Peeked empty socket" << std::endl;
			}
			if (client.peaksize == 0) {
				return;
			}
			std::cout << GREEN_TEXT << "> SOMETHING ON THE SOCKET" << RESET_COLOR << std::endl;
			// if (peekedSize >= MAX_HEADER_SIZE )//&& headerSize <= 0)
			// 	return; //!todo: not valid header, error, chiudi tutto (socket pieno ma nessun \r\n\r\n)
			// else if (peekedSize < MAX_HEADER_SIZE && headerSize <= 0)
			// 	return; // non-empty socket but no carriage return found.
			client.peaksize = _find_headers_limit(client, buffer);
			if (client.peaksize <= 0)
				return ;
			std::cout << "headers peeked" << std::endl;
			client.hpeaked = true;
			return ;
		}
		else 
		{
			std::cout << GREEN_TEXT << "READING REQUEST HEADERS" << RESET_COLOR << std::endl;
			int	headerSize = 0;
			char buffer[MAX_HEADER_SIZE];
			memset(buffer, 0, MAX_HEADER_SIZE);

			_init_structures(client);
			headerSize = recv(client.connection_socket.sock_fd, buffer, client.peaksize + 4, 0);
		
			if (headerSize == -1)
			{
				// std::cout << "Socket Empty" << std::endl;
				client.hpeaked = false;
				return;
			}
			else if (headerSize == 0)
			{
				// std::cout << "Connection Closed" << std::endl;
				client.hpeaked = false;
				return;
			}
			std::cout << BOLD_WHITE_TEXT << "####### MESSAGE ########" << std::endl;
			std::cout << buffer << std::endl;
			std::cout << "########################" << RESET_COLOR << std::endl;
			client.request->httpMessage.append(buffer);

			if (client.request->httpMessage.size() > MAX_REQUEST_SIZE) {
				client.setStatusCode(413);
				client.request->complete = true;
				client.hpeaked = false;
				throw (SizeExceeded());
			}
			size_t pos;
			if ((pos = client.request->httpMessage.find("\r\n\r\n")) != std::string::npos)
			{
				int result = _DetermineHttpType(client.request->httpMessage);

				if (result == REQUEST) {
					_parseHttpRequest(client.request->httpMessage, client);
					client.setStatusCode(200);
					client.request->complete = true;
					client.hpeaked = false;
				}
				else if (result == INVALID){
					client.setStatusCode(405); //! method not allowed
					std::cout << RED_TEXT << "client error id: " << client.id << RESET_COLOR << std::endl;
					client.request->complete = true;
					client.hpeaked = false;
					throw InvalidHttpMethodException();
				}
				else
					std::cout << "No Request Found" << std::endl;
			}
			else
				std::cout << "Request Incomplete" << std::endl;
		}
	} catch(const std::exception& e) {
		std::cout << RED_TEXT << e.what() << RESET_COLOR << std::endl;
	}
}
		
HttpRequest::~HttpRequest() {}

int HttpRequest::_DetermineHttpType(const std::string& httpMessage) {
	// Controllioamo se la stringa inizia con "GET", "POST" o "PUT" che è tipico di una richiesta HTTP
	if (httpMessage.find("GET ") == 0 || httpMessage.find("POST ") == 0 || httpMessage.find("DELETE ") == 0) {
		return REQUEST;
	} else { // Se non corrisponde a nessuno dei formati noti, messaggio HTTP non valido
		return INVALID;
	}
}

void HttpRequest::_parseHttpRequest(const std::string& httpRequest, sConnection& client) {
	// Troviamo la posizione della prima riga vuota che separa le intestazioni dal corpo
	size_t emptyLinePos = httpRequest.find("\r\n\r\n");

	if (emptyLinePos != std::string::npos) {
		// Dividiamo la richiesta in intestazioni e corpo
		std::string headerPart = httpRequest.substr(0, emptyLinePos + 2); //Kevin ha aggiunto + 2
		size_t start = 0;
		size_t end;

		while ((end = headerPart.find("\r\n", start)) != std::string::npos) {
			std::string line = headerPart.substr(start, end - start);
			size_t colonPos = line.find(':');

			if (colonPos != std::string::npos) {
				// Estraiamo e aggiungiamo la chiave e il valore alla mappa degli header
				std::string key = line.substr(0, colonPos);
				std::string value = line.substr(colonPos + 2); // +2 per saltare ": "
				client.request->headers[key] = value;
			}
			start = end + 2; // +2 per saltare "\r\n"
		}
		// La prima riga contiene metodo, URL e versione
		size_t firstLineEnd = headerPart.find("\r\n");
		std::string firstLine = headerPart.substr(0, firstLineEnd);
		size_t spacePos1 = firstLine.find(' ');
		size_t spacePos2 = firstLine.find(' ', spacePos1 + 1);

		if (spacePos1 != std::string::npos && spacePos2 != std::string::npos) {
			client.request->method = firstLine.substr(0, spacePos1);
			client.request->requestURL = firstLine.substr(spacePos1 + 1, spacePos2 - spacePos1 - 1);
			client.request->version = firstLine.substr(spacePos2 + 1);
		} else {
			client.setStatusCode(400); //! Bad request
			client.request->complete = true;
			throw HttpParseException();
		}

		// Verifichiamo se esiste un corpo (body) +4 per saltare "\r\n\r\n"
		if (emptyLinePos + 4 < httpRequest.length()) {
			client.request->body = httpRequest.substr(emptyLinePos + 4);
		}
		return;
	}
	// Parsing non riuscito
	client.setStatusCode(400); //! Bad request
	client.request->complete = true;
	throw HttpParseException();
}

void HttpRequest::_init_structures(sConnection &connection)
{
	if (!connection.request) {
		connection.request = new sRequest;
	}
	if (!connection.response) {
		connection.response = new sResponse;
	}
}

int HttpRequest::_find_headers_limit(sConnection &connection, const char *buffer)
{
	(void)connection;
	std::string	strbuffer(buffer);
	size_t	cr;

	cr = strbuffer.find("\r\n\r\n");

//	std::cout << MAGENTA_TEXT << "CR FOUND AT SIZE: " << cr << RESET_COLOR << std::endl;
	if (cr == std::string::npos)
		return 0;
	return (cr);
}
