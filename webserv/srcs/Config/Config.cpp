#include "../../libraries/includes.hpp"
#include "Config.hpp"

Config::Config(std::string const & path, std::vector<sServ> & servers) : \
		_servers(servers), _line(""), _lines_count(0), _position(0), _servers_count(0),  \
		_server_block(false), _server_brace(false), _location_block(false), \
		_location_brace(false), conf_file_ok(true) {

	this->_server_keywords.push_back("listen");
	this->_server_keywords.push_back("server_name");
	this->_server_keywords.push_back("root");
	this->_server_keywords.push_back("error_page");
	this->_server_keywords.push_back("client_max_body_size");
	this->_server_keywords.push_back("autoindex");
	this->_server_keywords.push_back("index");
	this->_server_keywords.push_back("return");
	this->_server_keywords.push_back("cgi_enable");

	this->_location_keywords.push_back("root");
	this->_location_keywords.push_back("error_page");
	this->_location_keywords.push_back("client_max_body_size");
	this->_location_keywords.push_back("methods");
	this->_location_keywords.push_back("autoindex");
	this->_location_keywords.push_back("index");
	this->_location_keywords.push_back("alias");
	this->_location_keywords.push_back("return");

	try {
		_read_config_file(path);
	}
	catch (ConfigParentException & e) {
		_error_close_ifstream();
		std::cout << RED_TEXT << e.configWhat(*this) << RESET_COLOR << std::endl;
	}
	catch (std::exception & e) {
		_error_close_ifstream();
		std::cout << RED_TEXT << e.what() << RESET_COLOR << std::endl;
	}
}

Config::~Config() {


}


//Read configuration file
void Config::_read_config_file(std::string path) {

	bool			http_block = false;
	bool			http_brace = false;
	std::string		line_read;

	this->_in_stream.open(path.c_str(), std::ios_base::in);
	_file_check();
	while (getline(_in_stream, line_read)) {
		this->_lines_count++;
		SANITIZE_STRING(line_read, this->_line);
		_position = 0;
		if (http_block == false) {
			if (this->_is_anything_inside_line() == false)
				continue;
			if (this->_is_that_word_now("http") == false)
				throw WrongDirectiveException();
			else {
				_position += std::string("http").length();
				http_block = true;
			}
		}
		if (http_block == true && http_brace == false) {
			if (this->_is_anything_inside_line() == false)
				continue;
			if (this->_is_that_word_now("{") == false)
				throw MissingBracesException();
			else {
				_position ++;
				http_brace = true;
				_read_http_block();
			}
		}
		if (this->_is_anything_inside_line() == false)
			continue;
		else
			throw WrongDirectiveException();
	}
	if (http_brace == false)
		throw NoServerException();
	_in_stream.close();
}


//Check if the file exists, can be open or is empty
void Config::_file_check() {

	if (!this->_in_stream.is_open()) {
		throw WrongConfFileException();
	}
	if (_in_stream.peek() == EOF) {
		_in_stream.close();
		throw WrongConfFileException();
	}
}


//Read every line of http block and call _parse_http_block to parse it
void Config::_read_http_block() {

	std::string	line_read;

	if (_parse_http_block() != 0)
		return;
	while (getline(this->_in_stream, line_read)) {
		this->_lines_count++;
		SANITIZE_STRING(line_read, this->_line);
		this->_position = 0;
		if (_parse_http_block() != 0)
			return;
	}
	_position = 0;
	throw MissingBracesException();
}


//Parse one line at a time in the http block
int Config::_parse_http_block() {

	while (true) {
		if (this->_server_block == false) {
			if (this->_is_anything_inside_line() == false)
				break;
			else if (_line[_position] == '}' && this->_servers_count > 0) {
				_position++;
				return 1;
			}
			else if (_line[_position] == '}' && _servers_count == 0)
				throw NoServerException();
			if (this->_is_that_word_now("server") == false)
				throw WrongDirectiveException();
			else {
				_position += std::string("server").length();
				_server_block = true;
			}
		}
		if (_server_block == true && _server_brace == false) {
			if (this->_is_anything_inside_line() == false)
				break;
			if (this->_is_that_word_now("{") == false)
				throw MissingBracesException();
			else {
				_position ++;
				_server_brace = true;
				_read_server_block();
				_server_block = false;
				_server_brace = false;
			}
		}
	}
	return 0;
}


//Read every line of server block and call _parse_server_block to parse it
void Config::_read_server_block() {

	std::string	line_read;
	sServ		serv_struct;

	this->_servers_count++;
	if (_parse_server_block(serv_struct)) //i.e. if server block is ended
		return;
	while (getline(this->_in_stream, line_read)) {
		this->_lines_count++;
		SANITIZE_STRING(line_read, this->_line);
		this->_position = 0;
		if (_parse_server_block(serv_struct)) //i.e. if server block is ended
			return;
	}
	_position = 0;
	throw MissingBracesException();
}

//Parse one line at a time in the server block
int	Config::_parse_server_block(sServ & serv_struct) {

	// std::cout << BLUE_TEXT << "_parse_server_block" << RESET_COLOR << std::endl;
	std::size_t	word_end_pos;
	std::size_t	path_end_pos;
	std::string	directive;
	std::string	location_path;
	sLocation	root_location;
	//std::vector<sServ>::iterator	it;
	sServ *		server_pointer;

	while (true) {
		if (this->_location_block == false) {
			if (this->_is_anything_inside_line() == false)
				return 0;
			else if (_line[_position] == '}') {
				_position++;
				break;
			}
			word_end_pos = _line.find(' ', _position) - 1;

			directive = _line.substr(_position, word_end_pos - _position + 1);
			if (_is_keywords(directive, this->_server_keywords) == true)
				_read_server_block_directive(serv_struct, directive);
			else if (directive == "location") {
				_position += std::string("location").length();
				_location_block = true;
			}
			else
				throw WrongDirectiveException();
		}
		if (_location_block == true && _location_brace == false) {
			if (this->_is_anything_inside_line() == false)
				return 0;
			if (this->_is_that_word_now("{") == true)
				throw WrongLocationPathException();
			else {
				_position = _line.find_first_not_of(' ', _position);
				if (_position == std::string::npos || _line[_position] == '{' || _line[_position] == '#')
					throw WrongLocationPathException();
				std::size_t space_pos = _line.find(' ', _position);
				std::size_t brace_pos = _line.find('{', _position);
				path_end_pos = ((space_pos < brace_pos) ? space_pos : brace_pos) - 1;
				location_path = _line.substr(_position, path_end_pos - _position + 1);
				if (!_is_location_path_ok(location_path))
					throw WrongLocationPathException();
				_position = brace_pos + 1;
				_location_brace = true;
				_read_location_block(serv_struct, location_path);
				_location_block = false;
				_location_brace = false;
			}
		}
	}
	root_location.loc_path = "/";
	_inherit_directives(serv_struct, root_location);
	serv_struct.locations.push_back(root_location); //add a default location to sServ struct

	server_pointer = _find_parent_server(serv_struct);
	if (server_pointer != NULL) {
		std::cout << "Domain names: " << vectorElementsList(serv_struct.domain_names) << std::endl;
		std::cout << "serv_struct.domain_names.size(): " << serv_struct.domain_names.size() << std::endl;
		if (serv_struct.domain_names.size() == 0)
			return 1;
		if (serv_struct.domain_names == server_pointer->domain_names)
			return 1;
		std::vector<std::string>::iterator	it_1;
		std::vector<std::string>::iterator	it_2;
		for (it_1 = server_pointer->domain_names.begin(); it_1 != server_pointer->domain_names.end(); ++it_1) {
			//found_same_name = false;
			for (it_2 = serv_struct.domain_names.begin(); it_2 != serv_struct.domain_names.end(); ++it_2) {
				if (*it_1 == *it_2) {
					//found_same_name = true;
					//return false
					throw ServersConflictException();
					//break;
				}
			}

			//if (found_same_name == false)
				//return true;
		}
		// if (_are_all_servernames_different(server_pointer->domain_names, serv_struct.domain_names) == true)	
		server_pointer->virtual_servers.push_back(serv_struct);
		// else
		// 	return 1;
	}
	else
		this->_servers.push_back(serv_struct);//add sServ struct to _servers vector
	//std::vector<sServ>::iterator it;
	// if
	// for (it = server_pointer->virtual_servers.begin(); it != server_pointer->virtual_servers.end(); ++it) {
	// 	std::cout << YELLOW_TEXT;
	//     it->print_virtual();
	// 	std::cout << RESET_COLOR << std::endl;
    // }
	return 1;
}


//Read every line of location block and call _parse_location_block to parse it
void Config::_read_location_block(sServ & serv_struct, std::string & location_path) {

	sLocation location_struct;
	std::string	line_read;

	serv_struct.directives_in_conf_file["location"] = true;
	this->_inherit_directives(serv_struct, location_struct);
	if (_parse_location_block(serv_struct, location_struct, location_path)) //i.e. if location block is ended
		return;
	while (getline(this->_in_stream, line_read)) {
		this->_lines_count++;
		SANITIZE_STRING(line_read, this->_line);
		this->_position = 0;
		if (_parse_location_block(serv_struct, location_struct, location_path)) //i.e. if location block is ended
			return;
	}
	_position = 0;
	throw MissingBracesException();
}


//Parse one line at a time in the location block
int Config::_parse_location_block(sServ & serv_struct, sLocation & location_struct, std::string & location_path) {

	std::size_t	word_end_pos;
	std::string	directive;

	while (true) {
			if (this->_is_anything_inside_line() == false)
				return 0;
			else if (_line[_position] == '}') {
				_position++;
				break;
			}
			word_end_pos = _line.find(' ', _position) - 1;
			directive = _line.substr(_position, word_end_pos - _position + 1);
			if (_is_keywords(directive, this->_location_keywords) == true) {
				_read_location_block_directive(location_struct, directive);
			}
			else
				throw WrongDirectiveException();
	}
	location_struct.loc_path = location_path;
	serv_struct.locations.push_back(location_struct); //add sLocation struct to locations vector	
	return 1;
}

bool	Config::_is_location_path_ok(std::string path) {

	return (path[0] == '/');
}

//Make sLocation struct inherit directives from sServ struct
void Config::_inherit_directives(sServ & serv_struct, sLocation & location_struct) {

	location_struct.root = serv_struct.root;
	location_struct.error_pages = serv_struct.error_pages;
	location_struct.client_max_body_size = serv_struct.client_max_body_size;
	location_struct.autoindex = serv_struct.autoindex;
	location_struct.index = serv_struct.index;
	location_struct.redirection = serv_struct.redirection;
}

//Close the ifstream and set conf_file_ok to false
void	Config::_error_close_ifstream() {
	
	this->_in_stream.close();
	this->conf_file_ok = false;
}
