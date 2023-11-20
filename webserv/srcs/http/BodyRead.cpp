#include "BodyRead.hpp"
#include <sys/socket.h>

int	timecheck(sConnection &ref)
{
	//KEVIN
	//size_t	size = MAX_BODY_SIZE;	// when there's a size in server, shall be taken from there.
	if (ref.takeTime == 0) {
		ref.takeTime++;
		ref.startBodyTime = std::time(NULL);
	}
	std::time_t currentTime = std::time(NULL);
	double elapsedTime = std::difftime(currentTime, ref.startBodyTime);
	// std::cout << "TIME DIFF: " << elapsedTime << "< -------------------" << std::endl;
	if (elapsedTime >= 60) {
		ref.setStatusCode(504);
		return 1;
	}
	return 0;
	//KEVIN
}

// void	freeExit(char *buffer)
// {
// 	if (buffer)
// 		delete buffer;
// 	return ;
// }

/**
 * @brief This class either reads a body or creates a temp
 * file for chunked bodies. if chunked is true, this class
 * is supposed to be called multiple times until a size 0
 * chunk is received.
 * 
 * @param ref 
 */
BodyRead::BodyRead(sConnection &ref) : _body_path("")
{
	if (ref.getStatusCode() >= 300)
		return;
	if (!ref.request)
		return ;
	if (ref.request->complete == false)
		return ;
	if (ref.request->body_expected == false)
		return ;
	if (timecheck(ref))
		return ;
	if (ref.request->chunked_expected == true)
	{
		if (ref.request->chunked_complete == false)
			_chunked_transfer(ref);
		return ;
	}
	
	std::cout << std::endl << GREEN_TEXT << "-BODY READ-" << RESET_COLOR << std::endl;
	size_t max = (*ref.request->bestLocation).client_max_body_size;
	size_t expected = ref.request->size_expected;
	if (expected > max)
	{
		std::cout << RED_TEXT << "Error: Content-Length exceeds MaxBodySize" << RESET_COLOR << std::endl;
		_exitError(ref, 413);
		return ;
	}

	int	bufferSize;
	char *buffer = ref.buffer;
	memset(buffer, 0, MAX_BODY_SIZE);

	bufferSize = recv(ref.connection_socket.sock_fd, buffer, expected + 1, 0);
	if (bufferSize == 0) {
		return;
	} else if (bufferSize == -1) {
		_exitError(ref, 400);
		return;
	}
	if (_checkSize(ref, bufferSize) && _exitError(ref, 413))
	{
		// freeExit(buffer);
		return ;
	}
	if (bufferSize > 0)
		ref.request->body.append(buffer, bufferSize);
	if (ref.request->body.size() < expected)
	{
		// std::cout << "returning because received only partial data" << std::endl;
		// freeExit(buffer);
		return ;
	}
	else if (ref.request->body.size() > expected)
	{
//		std::cout << "Peek size is: " << peekSize << std::endl;
		std::cout << "Too much stuff on the socket?" << std::endl;
		_exitError(ref, 413);
		// freeExit(buffer);
		return;
	}

	std::cout << GREEN_TEXT << "Parsing Body" << RESET_COLOR << std::endl;
	if (_parse_body(ref))
	{
		_exitError(ref, 400);
		std::cout << RED_TEXT << "Error Parsing Body" << RESET_COLOR << std::endl;
		// freeExit(buffer);
		return;
	}
	ref.request->body_complete = true;
	// std::cout << BLUE_TEXT << "Fine bodyread" << RESET_COLOR << std::endl;
	
	// freeExit(buffer);
	return;
}

BodyRead::~BodyRead()
{
	//
}

//curl -X POST -H "Transfer-Encoding: chunked" --data-binary @reg.txt 127.0.0.1:8080/gallery/
int	BodyRead::_chunked_transfer(sConnection &ref)
{

	std::cout << GREEN_TEXT << " - CHUNKED READ - " << RESET_COLOR << std::endl;

	char	*buffer = ref.buffer;
	memset(buffer, 0, MAX_BODY_SIZE);

	if (ref.request->c_hexpeeked == false)
	{
//		size_t max = (*ref.request->bestLocation).client_max_body_size;
		int		peekSize;
		size_t  cr;
		std::string	pstring;
		std::string	hexsize;

		peekSize = recv(ref.connection_socket.sock_fd, buffer, 50, MSG_PEEK);
		if (peekSize == 0) {
			return 0;
		} else if (peekSize == -1) {
			// ref.request->c_hexpeeked = true;
			// _exitError(ref, 401);
			// ref.errorClose = true;
			return 0;
		}
		
		pstring = buffer;
		if ((cr = pstring.find("\r\n")) == std::string::npos)
			return 0;
		hexsize = pstring.substr(0, cr);
		ref.request->hexsize = hexsize.size();
		HEX_TO_INT(hexsize, ref.request->chunksize);
		// std::cout << "peek:" << std::endl << pstring << std::endl << "CHUNKSIZE: " << ref.request->chunksize << std::endl;
		ref.request->c_hexpeeked = true;
		return 0;
	}
	else
	{
		int		size;
		int		chsiz = ref.request->chunksize;
		int		hexsiz = ref.request->hexsize;

		if (ref.request->chunksize == 0)
		{
			// std::cout << "chunksize is 0" << std::endl;
			size = recv(ref.connection_socket.sock_fd, buffer, 5, 0);
			ref.request->chunked_complete = true;
			ref.request->body_complete = true;
			ref.request->c_hexpeeked = false;
			return 0;
		}
		size = recv(ref.connection_socket.sock_fd, buffer, (hexsiz + chsiz + 4) , 0);
		if (size <= 0)
		{
			ref.request->c_hexpeeked = false;
			return 1;
		}
		std::string	bstring(buffer, hexsiz + chsiz + 4);

		// std::cout << BLUE_TEXT << "####" << std::endl << bstring << "####" << RESET_COLOR << std::endl;
		bstring = bstring.substr(hexsiz + 2, chsiz);
		// std::cout << CYAN_TEXT << "####" << std::endl << bstring << "####" << RESET_COLOR << std::endl;

		// std::cout << "We have read a chunk of size: " << chsiz << std::endl;

		std::stringstream sname;
		std::string			name;
		sname << ref.id;
		name = sname.str();
		name = "./www/temp/" + name;
		// std::cout << "creating a file to append to : " << name << std::endl;

		std::ofstream file(name.c_str(), std::ios::app | std::ios::binary);
    	if (file) {
			// std::cout << "writing chunk to file: " << name << std::endl;
			ref.request->temp_file = name;
			file.write(bstring.c_str(), bstring.size());
    	    file.close();
    	    // std::cout << "Buffer written to file successfully." << std::endl;
    	} else {
    	    std::cerr << "Failed to open the file for writing." << std::endl;
			_exitError(ref, 500);
			ref.request->c_hexpeeked = false;
			return 1;
    	}

		ref.request->c_hexpeeked = false;
	}
	return 0;
}

// parses body in search of a content-type header. if such header is found, a separator field is found.
// 
int	BodyRead::_parse_body(sConnection &ref)
{
	std::string	ConType(ref.request->headers["Content-Type"]);
	std::string Separator;
	if (ConType.size() != 0)
	{
		//! TEMPORARY SOLUTION: IF ORDER CHANGES OR SEPARATOR MISSING, MIGHT BREAK.
		size_t	sc = ConType.find(";");
		if (sc != std::string::npos)
		{
			Separator = ConType.substr(sc + 1);
			ConType = ConType.substr(0, sc);
			size_t	eq = Separator.find("=");
			Separator = Separator.substr((eq + 1));
			Separator = "--" + Separator;
		}
		//!

//		std::cout << "Content Type: " << ConType << std::endl << "Separator: " << Separator << std::endl;
	if (ConType == "multipart/form-data")
		return (_multipart_formData(ref, Separator));
	}
	return 0;
}

/**
 * @brief this function splits the body in parts and then pass them to a parser
 * 
 * @param ref 
 * @param sep 
 * @return int 
 */
int	BodyRead::_multipart_formData(sConnection &ref, std::string sep)
{
	std::cout << MAGENTA_TEXT << "ENTERING MULTIPART PARSING" << RESET_COLOR << std::endl;
	
	if (sep.size() <= 0)
	{
		std::cout << "Multipart true, but no separator provided" << std::endl;
		return (_exitError(ref, 400));
	}
	// std::cout << CYAN_TEXT << "#" << RESET_COLOR << std::endl;
	// std::cout << ref.request->body << std::endl;
	// std::cout << CYAN_TEXT << "#" << RESET_COLOR << std::endl;

	size_t	match, begin = 0;
	std::string&	bod = ref.request->body;
	std::vector<std::string>	parts;

//	std::cout << "body size: " << bod.size() << std::endl;
	while ((match = bod.find(sep, begin)) != std::string::npos && match <= bod.size())
	{
//		std::cout << "begin: " << begin << " match: " << match << " bodysize: " << bod.size() << std::endl;
		if (match != begin)
		{
			std::string	part(bod.substr(begin, (match - begin)));
			part = part.substr(sep.size());
			parts.push_back(part);
		}
		begin = match + 1;
		if (begin > bod.size())
			break;
		// sleep (1);
	}

	_parse_parts(ref, parts);
	return 0;
}

/**
 * @brief this function split each part in header and content. It then declares a dataForm
 * whose constructor parse the headers and content strings passed to it, and fill itself.
 * dataForms are then pushed to request.forms vector.
 * 
 * @param ref 
 * @param parts 
 * @return int 
 */
int BodyRead::_parse_parts(sConnection &ref, std::vector<std::string> &parts)
{
	(void)ref;
	for (size_t x = 0; x < parts.size(); x++)
	{
		std::string	head, body;
		size_t	sep = parts[x].find("\r\n\r\n");

		head = parts[x].substr(0, (sep + 2));
	
		body = parts[x].substr(sep + 4);
		body = body.substr(0, body.size() - 2);

//		std::cout << "#### Part " << x << " ####" << std::endl;
//		std::cout << head << std::endl << "#" << std::endl << body << std::endl;

		// std::cout << MAGENTA_TEXT << "#" << RESET_COLOR << std::endl;
		// std::cout << body << std::endl;
		// std::cout << MAGENTA_TEXT << "#" << RESET_COLOR << std::endl;
	
		dataForm	form(head, body);
	//	form.print();
	
	//	std::cout << BOLD_CYAN_TEXT;
	//	form.print();
	//	std::cout << RESET_COLOR;
		ref.request->forms.push_back(form);

	}
	return 0;
}

int BodyRead::_exitError(sConnection &ref, int Error)
{
	ref.setStatusCode(Error);
	ref.request->body_expected = false;
	ref.request->body_complete = true;
	return 1;
}

int BodyRead::_checkSize(sConnection &ref, int BufferSize)
{
	if (ft_stoi(ref.request->headers["Content-Length"]) < (BufferSize + (int)ref.request->body.size()))
		{
			std::cout << RED_TEXT << "Received more data than expected." << RESET_COLOR << std::endl;
			return 1;
		}
	return 0;
}

/**
 * @brief SO_SNDBUF SO_RCVBUF are the flags to check either snd buffer and receive buffer of the socket
 * 
 * @param ref 
 * @param FLAG 
 * @return int 
 */
int BodyRead::_printSocketBoundaries(sConnection &ref, int FLAG)	
{
	// EVALUATE SOCKET HARD LIMITS:
	int bufferSiz;
	socklen_t option_len = sizeof(bufferSiz);
//  if (getsockopt(ref.connection_socket.sock_fd, SOL_SOCKET, SO_SNDBUF, &bufferSiz, &option_len) == -1) {
	if (getsockopt(ref.connection_socket.sock_fd, SOL_SOCKET, FLAG, &bufferSiz, &option_len) == -1) {
		std::cerr << RED_TEXT << "Error getting receive buffer size" << RESET_COLOR << std::endl;

	}
	// std::cout << "SOCKET Receive buffer LIMIT: " << bufferSiz << " bytes" << std::endl;
	return 0;
}
