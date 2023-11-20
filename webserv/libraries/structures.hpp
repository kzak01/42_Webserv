#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

# include "includes.hpp"
//# include "../srcs/Config/utils/utils.hpp"

// HTTP METHODS STRUCT
struct sHttpMethods
{
	bool get;
	bool post;
	bool del;

	sHttpMethods();
	sHttpMethods(sHttpMethods const & other);
	sHttpMethods & operator=(sHttpMethods const & other);
	~sHttpMethods();
};

std::ostream & operator<<(std::ostream & o, sHttpMethods const & http_methods);

struct sSocket {
	sockaddr_in	sockaddr;
	int			sock_fd;
	socklen_t	sockaddr_len;
};

struct dataForm {
	dataForm(std::string head, std::string content) : _headerpart(head), _content(content), _contentType(""), _contentDisp(""), _contentSyze(content.size()), _name(""), _filename("")
	{
		splitheaders(head);
		parseheaders();
	};
	//~dataForm();

	std::vector<std::string> 	_headers;
	std::string					_headerpart;
	std::string					_content;

	std::string					_contentType;
	std::string					_contentDisp;
	int							_contentSyze;
	std::string					_name;
	std::string					_filename;

	void						splitheaders(std::string head)
	{
		size_t match, begin = 1;

//		std::cout << "SPLITTING HEADERS:" << std::endl << head << std::endl;
		while ((match = head.find("\n", begin)) != std::string::npos)
		{
			std::string str = head.substr(begin, (match - begin));
//			std::cout << ">" << str << std::endl;
			_headers.push_back(str);
			begin = match + 1;
		}
	}

	void						parseheaders() 
	{
//		std::map<std::string, std::string>	directives;
		size_t match;//, begin = 0;//, eq = 0;
		for (size_t x = 0; x < _headers.size(); x++)
		{
			std::string	str1;
			std::string str2;
//			std::cout << "x: " << x << std::endl;
//			std::cout << _headers[x] << std::endl;
			if ((match = _headers[x].find(":")) != std::string::npos)
			{
				str1 = _headers[x].substr(0, match + 1);
				str2 = _headers[x].substr(match + 2);

//				std::cout << "$$$HEADERS: " << std::endl << str1 << std::endl << str2 << std::endl;

				if (str1 == "Content-Disposition:")
				{
					size_t sc = str2.find(";");
					if (sc != std::string::npos)
						_contentDisp = str2.substr(0, sc);
					
					size_t mtc = 0, dq;
					mtc = str2.find("name=\"");
					if (mtc != std::string::npos)
					{
						mtc +=  6;
						dq = str2.find("\"", mtc);
						_name = str2.substr(mtc, (dq - mtc));
					}
					
					mtc = str2.find("filename=\"");
					if (mtc != std::string::npos)
					{
						mtc +=  10;
						dq = str2.find("\"", mtc);
						_filename = str2.substr(mtc, (dq - mtc));
					}
				}
				if (str1 == "Content-Type:")
					_contentType = str2;
			}
		}
	}

	void						print() {
		std::cout << "# DATA FORM #" << std::endl;
		std::cout << "Content type: " << _contentType << std::endl;
		std::cout << "Content syze: " << _contentSyze << std::endl;
		std::cout << "Name: " << _name << std::endl;
		std::cout << "Filename: " << _filename << std::endl;
		std::cout << _content << std::endl;
		std::cout << "###############################" << std::endl;
	}
};

// LOCATION BLOCK STRUCT
struct sLocation
{ //se si aggiunge una variabile, aggiungerla anche a operator= e operator<< in sLocation.cpp

	std::string					loc_path;

	//DIRECTIVES
	std::string 				root;
	std::map<int, std::string>	error_pages;
	//std::string					client_max_body_size;
	std::size_t						client_max_body_size;
	sHttpMethods				methods;
	bool						autoindex;
	std::string					index;
	std::string					alias;
	std::pair<int, std::string>	redirection; //return directive

	std::map<std::string, bool> directives_in_conf_file;//Indicate which directives are in conf file (compared to ones that are not and have default values)

	sLocation();
	sLocation(sLocation const & other);
	sLocation & operator=(sLocation const & other);
	~sLocation();

	void print();
};

std::ostream & operator<<(std::ostream & o, sLocation const & serv);

struct sBase {
	int		socket_type;
};

// SERVER STRUCT
struct sServ : public sBase{ //se si aggiunge una variabile, aggiungerla anche a operator<< in sServ.cpp
//	int			sockFd;
	sSocket		_listening_socket;
	epoll_event	_event;

	//DIRECTIVES
	std::vector<std::string>    domain_names; //server_name directive
	std::string					ip_address;
	std::string					port;
	std::string 				root;
	std::map<int, std::string>	error_pages;
	//std::string				client_max_body_size;
	std::size_t					client_max_body_size;
	bool						autoindex;
	std::string					index;
	std::pair<int, std::string>	redirection; //return directive
	std::vector<sLocation>		locations;
	std::vector<std::string>	cgi_enable;

	std::vector<sServ> 			virtual_servers;

	std::map<std::string, bool> directives_in_conf_file;//Indicate which directives are in conf file (compared to ones that are not and have default values)

	sServ();
	~sServ();

	void print();
	void print_virtual();
};

std::ostream & operator<<(std::ostream & o, sServ const & serv);

struct sRequest {			//! esempio con localhost:9000/www/html/index.html?ciao=asd/bella=zi
	sRequest();
	~sRequest();
	bool									parsed;
	bool									is_cgi;
	std::string								httpMessage; // messaggio intero preso a pezzi
	bool									complete; // finito di leggere o serve un altro giro
	bool									answered;
	std::string								method; // GET, POST, DELETE //! GET
	std::string								requestURL; // tutta la url //! /www/html/index.html?ciao=asd/bella=zi
	std::string								path_file; // la url senza query //! /www/html/index.html
	std::vector<std::string>				folders; // le cartelle del URL //! www html
	std::string								fileName; // solo il file nella URL //! index.html
	std::string								extension; // estensione del file se esiste //! html
	std::string								queryName; // le query, dopo il '?' nella URL //! ciao=asd/bella=zi
	std::map<std::string, std::string>		queryMap; // querry divisa per: chiave=valore&... //! ciao[asd] bella[zi]
	std::string								version; // HTTP1.1
	std::map<std::string, std::string>		headers;

	//BODY
	std::string								body;
	bool									body_expected;
	bool									body_complete;
	std::string								body_path;
	std::vector<dataForm>					forms;
	int										size_expected;
	bool									chunked_expected;
	bool									c_hexpeeked;
	int										chunksize;
	int										hexsize;
	//#CATIA 
	bool									chunked_complete;	//chunked ha finito di operare
	std::string								temp_file;			//se chunked ha finito, un file con questo nome dovrebbe essere stato creato

	std::vector<sLocation>::iterator		bestLocation; // migliore corrispondenza con le location, di base c'e' /
	std::string								rootInLocation; // la root nella location //! es /www/html
	std::string								rootPlusURL; // path completo root + Url; //! www/html/www/html/index.html
	void printRequest() { // per debug
		if (complete == false)
				return;
		std::cout << "-----------------------------------\n" << "Method: " << method << "\n"
						<< "RequestUrl: " << requestURL << "\n"
						<< "Version: " << version
						<< std::endl;
		for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
				std::cout << it->first << ": " << it->second << std::endl;
		}
		std::cout << "Body: " << body << "\n" << "-----------------------------------" << std::endl;
	}
};

struct sResponse 
{
	sResponse() {
		autoindex_response = false;
		cgi_response = false;
		complete = false;
		status_code = 0;
		file_length = 0;
		file_type = "";
		domain = "";
		file_name = "";
		file_path = "";
		error_flag = false;
		header = "";
		memset(&file_stat, 0, sizeof(file_stat));

	}
	~sResponse() {
		if ((cgi_response == true || autoindex_response == true) && file_path.size() > 0)
		{
			debug_printing(E_WARNING, "removing file -> " + file_path);
			remove(file_path.c_str());
		}
	}
	std::vector<std::string> _headers;
	std::string	header;
	bool		autoindex_response;
	bool		cgi_response;
	bool		complete;
	bool		error_flag;
	int			status_code;
	int			file_length;

	std::string	file_type;
	std::string	domain;
	std::string file_name;
	std::string	file_path;
	struct stat	file_stat;
};




struct sConnection : public sBase
{
	sConnection() { cgi_executed = false; errorClose = false; peaksize = 0; hpeaked = false; server = NULL; request = NULL; response = NULL; takeTime = 0; takeConnectionTime = 0;}

	bool		hpeaked;
	int			peaksize;
	//bool		status;
	bool		errorClose;
	bool		cgi_executed;
	
	int			id;
	sSocket		connection_socket;
	sServ		*server;
	sRequest	*request;
	sResponse	*response;
	char		buffer[MAX_BODY_SIZE];

	bool operator==(const sConnection& other) const {
		return this->id == other.id; // Compare based on a relevant field (e.g., id)
	}

	std::time_t	startBodyTime;
	size_t		takeTime;
	std::time_t	startConnectionTime;
	size_t		takeConnectionTime;

	int getStatusCode() {
		return (this->response->status_code);
	}
	void setStatusCode(int code) {
		this->response->status_code = code;
	}

	std::string _StatusString(int statusCode) {
		switch (statusCode) {
		case 100: return "100 Continue";
		case 101: return "101 Switching Protocols";
		case 200: return "200 OK";
		case 201: return "201 Created";
		case 204: return "204 No Content";
		case 301: return "301 Moved Permanently";
		case 302: return "302 Found";
		case 304: return "304 Not Modified";
		case 400: return "400 Bad Request";
		case 401: return "401 Unauthorized";
		case 403: return "403 Forbidden";
		case 404: return "404 Not Found";
		case 405: return "405 Method Not Allowed";
		case 413: return "413 Request Entity Too Large";
		case 500: return "500 Internal Server Error";
		case 501: return "501 Not Implemented";
		case 503: return "503 Service Unavailable";
		case 504: return "504 Gateway Timeout";
		case 505: return "505 HTTP Version Not Supported";
		default: return "Unknown Status";
	}
}
};


#endif // STRUCTURES_HPP
