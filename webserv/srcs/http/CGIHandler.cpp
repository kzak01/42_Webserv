#include "CGIHandler.hpp"

typedef std::map<std::string, std::string> rmap;

// ----------------------------------------------------------------------------
// --- Auxiliary methods

char	*to_char_str(std::string str)
{
	char	*result = new char[str.size() + 1];

	for (std::size_t c = 0; c < str.size() + 1; c++)
		result[c] = str[c];

	return (result);
}

bool request_is_cgi(sRequest &req, sConnection &_conn)
{
	std::vector<std::string>::iterator it; _conn.server->cgi_enable.begin();
	for (it = _conn.server->cgi_enable.begin(); true; it++) {
		if (it == _conn.server->cgi_enable.end()) {
			return false;
		}
		if ((*it).substr(1, (*it).length()) == req.extension) {
			break;
		}
	}
	if (req.requestURL.find("cgi-bin/") != std::string::npos &&
			(req.requestURL.find(".py") != std::string::npos ||
			req.requestURL.find(".pl") != std::string::npos ||
			req.requestURL.find(".rb") != std::string::npos))
		return true;
	return false;
}

bool cgi_should_be_executed(sConnection & conn)
{
	conn.request->is_cgi = request_is_cgi(*conn.request, conn);
	conn.response->cgi_response = conn.request->is_cgi;
	if (conn.request->is_cgi)
		return true;
	return false;
}

static void	set_internal_error(sConnection & conn, sRequest & req, sResponse & res)
{
	debug_printing(E_ERROR, "CGI: Internal Server Error");
	conn.setStatusCode(500);
	res.cgi_response = false;
	req.is_cgi = false;
}

rmap subdivide_body(std::string str)
{
	rmap my_map;
	std::string delimiter = "\r\n\r\n";

	size_t pos = str.find(delimiter);
	if (pos == std::string::npos)
		return my_map;

	pos += delimiter.size();

	my_map["headers"] = str.substr(0, pos);
	my_map["body"] = str.substr(pos);

	return my_map;
}

std::vector<std::string> get_headers(std::string str)
{
	std::string	delimiter = "\r\n";
	std::vector<std::string> headers;
	u_long	spos = 0;
	u_long	epos = str.find(delimiter);

	while (epos != std::string::npos)
	{
		headers.push_back(str.substr(spos, epos - spos));
		spos = epos + delimiter.size();
		epos = str.find(delimiter, spos);
	}

	headers.push_back(str.substr(spos));

	return headers;
}

// ----------------------------------------------------------------------------
// --- Constructors/Destructors

CGI_Handler::~CGI_Handler(void) 
{ 
	// delete CGI_PATH; 
}

CGI_Handler::CGI_Handler(sConnection & connection) : _conn(connection)
{
	if (connection.cgi_executed == true)
		return ;
	if ((!connection.request || connection.request->complete == false)
			|| connection.getStatusCode() >= 300
			|| !(cgi_should_be_executed(connection)))
		return ;
	connection.cgi_executed = true;
	std::cout << std::endl << GREEN_TEXT << "-CGI-" << RESET_COLOR << std::endl;

	static u_long file_id;

	file_id++;
	connection.response->file_path = "temp_files/" + numToString(file_id) + ".tp";

	this->init_env(connection, *connection.request);
	debug_printing(E_INFO, "CGI: Filled execve env with no problem");
	this->fill_response(
		connection, 
		*connection.response, 
		this->request(connection.response->file_path)
	);
}

// ----------------------------------------------------------------------------
// --- Env Generation

void	CGI_Handler::env(std::string key, std::string value)
{
	this->_env[key] = value;
}

char**	CGI_Handler::get_env() const
{
	char	**env = new char*[this->_env.size() + 1];
	int		c = 0;

	for (std::map<std::string, std::string>::const_iterator i = this->_env.begin();
			i != this->_env.end(); i ++)
	{
		env[c++] = to_char_str(i->first + "=" + i->second);
		debug_printing(E_DEBUG, env[c-1]);
	}
	env[c] = NULL;

	return (env);
}

void	CGI_Handler::put_query_variables_into_env(std::map<std::string, std::string> query){
	for (std::map<std::string, std::string>::const_iterator i = query.begin();
			i != query.end(); i++)
		this->env(i->first, i->second);
}

void	CGI_Handler::init_env(sConnection & connection, sRequest & request){

	this->_body = request.body;

	this->env("CONTENT_TYPE", "application/x-www-form-urlencoded");
	this->env("CONTENT_LENGTH", numToString(this->_body.size()));
	this->env("PATH_INFO", "/cgi-bin/" + request.fileName);
	this->env("SERVER_PORT", connection.server->port); 
	this->env("SERVER_SOFTWARE", "Webserve/1.0");
	this->env("QUERY_STRING", request.queryName);
	this->env("REQUEST_METHOD", request.method);
	this->env("REQUEST_URI", request.path_file);
	this->env("SCRIPT_NAME", request.fileName);
	this->env("GATEWAY_INTERFACE", "CGI/1.1");
	this->env("SERVER_PROTOCOL", "HTTP/1.1");
	this->env("REDIRECT_STATUS", "200");
	this->env("BODY", this->_body);

	// empty values
	this->env("PATH_TRANSLATED", "");
	this->env("REMOTE_IDENT", "");
	this->env("REMOTE_ADDR", "");
	this->env("REMOTE_HOST", "");
	this->env("REMOTE_USER", "");
	this->env("SERVER_NAME", "");
	this->env("AUTH_TYPE", "");

	// query string variables
	this->put_query_variables_into_env(request.queryMap);
}

// ----------------------------------------------------------------------------
// --- Request aux

char *CGI_Handler::file_path(const char *file)
{
	// generate path for execve
	char *path = new char[strlen(file) + strlen(CGI_PATH) + 2];

	int i = 0;
	int j = 0;
	while (CGI_PATH[i])
		path[j++] = CGI_PATH[i++];
	path[j++] = '/';
	i = 0;
	while (file[i])
		path[j++] = file[i++];
	path[j] = '\0';

	return (path);
}


char	**CGI_Handler::generate_argv(void){

	std::string	file = this->_env["SCRIPT_NAME"];
	char **argv = new char*[2];
	char *file_name = to_char_str(file);

	argv[0] = file_name;
	argv[1] = NULL;

	return (argv);
}

// ----------------------------------------------------------------------------
// --- Request
//			- redirect cgi output to the body response we should send

std::string CGI_Handler::request(std::string file_path)
{
	std::string 	html_body = "";
	
	FILE			*tmp_files[3];
	int			sc[3];
	long		fd[3];

	char		buffer[CGI_BUFFER_SIZE];
	long int	read_ = 0;

	char 		**argv = this->generate_argv();
	std::string	path = "./www/html" + this->_env["REQUEST_URI"];
	char		**environment = this->get_env();

	for (int i = 0; i < 3; i++)
	{
		sc[i] = dup(i);		// standard copy
		tmp_files[i] = tmpfile();	// tmp_files
		fd[i] = fileno(tmp_files[i]); // fd dei tmp_files
	}

	debug_printing(E_INFO, "About to fork");
	switch (fork()){
	case -1:
		std::cout << "fork failure\n";
		return ("Status: 500\r\n\r\n");
		break ;

	case 0: // child

		for (int i = 0; i < 3; i++) { dup2(fd[i], i); }

		execve(path.c_str(), argv, environment);

		std::cout << "execve failure\n";
		{
			int err = write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15); // maybe not on std error
			if (err == -1)
			{
				this->_conn.setStatusCode(500);
				debug_printing(E_ERROR, "CGI-Child: cannot write into file");
				break ;
				}
				else if (err == 0)
				debug_printing(E_INFO, "CGI-Child: nothing was written into file");
			else
				debug_printing(E_INFO, "CGI-Child: written " + numToString(err) + " bytes.");
		}
		break ;

	default : // parent
		float timestamp = 0;
		while (1)
		{
			if (timestamp > MAX_CGI_TIME)
				break ;
			waitpid(-1, NULL, WNOHANG);
			usleep(1000);
			timestamp += 1000;
		}
		debug_printing(E_INFO, "CGI: Child died");
		lseek(fd[STDOUT_FILENO], 0, SEEK_SET);
		read_ = read(fd[STDOUT_FILENO], buffer, sizeof buffer);
		while (read_ > 0)
		{
			for (long int i = 0; i < read_; i++)
				html_body.push_back(buffer[i]); // write in a temp file of output
			memset(buffer, 0, sizeof buffer);
			read_ = read(fd[STDOUT_FILENO], buffer, sizeof buffer);
		}
		if (read_ == -1)
		{
			debug_printing(E_DEBUG, "CGI: reading script output failure");
			this->_conn.setStatusCode(500);
			break ;
		}
		else if (read_ == 0)
			debug_printing(E_INFO, "CGI: successfully read script output");
		// Write to Temp file the html body
		debug_printing(E_INFO, "CGI: Filling temporary file with only body");
		debug_printing(E_DEBUG, html_body);

		if (html_body.find("\r\n\r\n") == std::string::npos)
			break ;

		unsigned long pos = html_body.find("\r\n\r\n");
		std::ofstream	file(file_path.c_str());
		file << html_body.substr(pos + 2);
		debug_printing(E_INFO, "Header+Body size -> " + numToString(html_body.size()));
		file << "\0";
		file.close();
	}

	for (int i = 0; i < 3; i++)
	{
		fclose(tmp_files[i]);
		close(fd[i]);
		dup2(sc[i], i);
	}
	
	for (size_t i = 0; environment[i]; i++)
		delete[] environment[i];
	delete[] environment;
	delete[] argv[0];
	delete[] argv;
	
	return (html_body);
}

void	CGI_Handler::fill_response(sConnection & conn, sResponse & res, std::string body)
{
	rmap my_map = subdivide_body(body);

	if (this->_conn.getStatusCode() >= 300
			|| body.find("Status: 500\r\n\r\n") != std::string::npos
			|| body.size() == 0)
		return set_internal_error(conn, *conn.request, res);

	res.header = my_map["headers"];

	struct stat fileStat;
	stat(res.file_path.c_str(), &fileStat);
	res.file_stat = fileStat;
	// res.file_stat.st_size = my_map["body"].size();
	
	debug_printing(E_INFO, "CGI: Filled headers response");
	
	res.complete = true;
	// conn.request->answered = true;
	debug_printing(E_INFO, "CGI: End");
}
