#include "../../libraries/includes.hpp"
#include "Config.hpp"

//Read all the directives in a server block, outside location blocks
void	Config::_read_server_block_directive(sServ & serv_struct, std::string directive) {

	std::size_t	semicolon_pos = std::string::npos;

	this->_position += std::string(directive).length();
	_position = _line.find_first_not_of(" ", _position);
	if (_position == std::string::npos || _line[_position] == '}' || _line[_position] == ';')
		throw WrongValuesException();
	semicolon_pos = _line.find(';', _position);
	if (semicolon_pos == std::string::npos)
		throw MissingSemicolonException();
	if (directive == "root" || directive == "index")
		_store_simple_directive_value<sServ>(serv_struct, semicolon_pos - 1, directive);
	else if (directive == "listen")
		this->_store_listen_values(serv_struct, semicolon_pos - 1);
	else if (directive == "server_name")
		this->_store_server_name_values(serv_struct, semicolon_pos - 1);
	else if (directive == "error_page")
		this->_store_error_page_values<sServ>(serv_struct, semicolon_pos - 1);
	else if (directive == "client_max_body_size")
		this->_store_client_max_body_size_value<sServ>(serv_struct, semicolon_pos - 1);
	else if (directive == "autoindex")
		this->_store_autoindex_value<sServ>(serv_struct, semicolon_pos - 1);
	else if (directive == "return")
		this->_store_return_values<sServ>(serv_struct, semicolon_pos - 1);

	else if (directive == "cgi_enable")
		this->_store_cgi_enable_values(serv_struct, semicolon_pos - 1);
	else
		throw WrongDirectiveException();
	_position = semicolon_pos + 1;
	std::size_t after_semicol_char_pos = _line.find_first_not_of(" ", _position);
	if (after_semicol_char_pos != std::string::npos && _line[after_semicol_char_pos] != '#')
		throw CharAfterSemicolonException();
}

//Read all the directives in a location block
void	Config::_read_location_block_directive(sLocation & location_struct, std::string directive) {

	std::size_t	semicolon_pos = std::string::npos;

	this->_position += std::string(directive).length();
	_position = _line.find_first_not_of(" ", _position);
	if (_position == std::string::npos || _line[_position] == '}' || _line[_position] == ';')
		throw WrongValuesException();
	semicolon_pos = _line.find(';', _position);
	if (semicolon_pos == std::string::npos)
		throw MissingSemicolonException();
	if (directive == "root" || directive == "index")
		_store_simple_directive_value<sLocation>(location_struct, semicolon_pos - 1, directive);
	else if (directive == "error_page")
		this->_store_error_page_values<sLocation>(location_struct, semicolon_pos - 1);
	else if (directive == "client_max_body_size")
		this->_store_client_max_body_size_value<sLocation>(location_struct, semicolon_pos - 1);
	else if (directive == "autoindex")
		this->_store_autoindex_value<sLocation>(location_struct, semicolon_pos - 1);
	else if (directive == "return")
		this->_store_return_values<sLocation>(location_struct, semicolon_pos - 1);
	else if (directive == "methods")
		this->_store_methods_values(location_struct, semicolon_pos - 1);
	else if (directive == "alias")
		this->_store_alias_value(location_struct, semicolon_pos - 1);
	else
		throw WrongDirectiveException();
	_position = semicolon_pos + 1;
	std::size_t after_semicol_char_pos = _line.find_first_not_of(" ", _position);
	if (after_semicol_char_pos != std::string::npos && _line[after_semicol_char_pos] != '#')
		throw CharAfterSemicolonException();
}


//Store listen directive values
void Config::_store_listen_values(sServ & serv_struct, std::size_t values_end) {

	std::string							ip_string = "";
	std::string							port_string = "";
	std::size_t							colon_pos = std::string::npos;
	//std::vector<sServ>::const_iterator	it;

	serv_struct.directives_in_conf_file["listen"] = true;
	colon_pos = this->_line.find(':', this->_position);
	if (colon_pos == std::string::npos)
		throw WrongValuesException();
	ip_string = _line.substr(_position, colon_pos - _position);
	this->_cleanTrim(ip_string);
	serv_struct.ip_address = ip_string;
	_position = colon_pos + 1;
	port_string = _line.substr(_position, values_end - _position + 1);
	_cleanTrim(port_string);
	if (port_string.length() > PORT_MAX_LENGTH)
		throw WrongValuesException();
	// for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
	// 	if (it->port == port_string)
	// 		//throw WrongValuesException();
	// }

	serv_struct.port = port_string;
}


//Split the values of a directive
void Config::_split_directive_values(std::size_t values_end, std::vector<std::string> & vec) {

	std::size_t	space_pos = 0;
	std::vector<std::string>::const_iterator it;

	space_pos = this->_line.find(' ', this->_position);
	
	while (space_pos != std::string::npos && space_pos <= values_end) {
		vec.push_back(_line.substr(_position, space_pos - _position));
		_position = space_pos + 1;
		_position = _line.find_first_not_of(' ', _position);
		space_pos = this->_line.find(' ', this->_position);
	}
	if (_position <= values_end)
		vec.push_back(_line.substr(_position, values_end - _position + 1));
}


//Store server_name directive values
void Config::_store_server_name_values(sServ & serv_struct, std::size_t values_end) {

	std::vector<std::string>  values_vector;
	std::vector<std::string>::const_iterator it;

	serv_struct.directives_in_conf_file["server_name"] = true;
	this->_split_directive_values(values_end, values_vector);
	serv_struct.domain_names.clear();
	for (it = values_vector.begin(); it != values_vector.end(); ++it)
		serv_struct.domain_names.push_back(*it);
}

//Store cgi_enable directive values
void	Config::_store_cgi_enable_values(sServ & serv_struct, std::size_t values_end) {

	std::vector<std::string>  values_vector;
	std::vector<std::string>::const_iterator it;

	serv_struct.directives_in_conf_file["cgi_enable"] = true;
	this->_split_directive_values(values_end, values_vector);
	serv_struct.cgi_enable.clear();
	for (it = values_vector.begin(); it != values_vector.end(); ++it)
		serv_struct.cgi_enable.push_back(*it);
}

//Store methods directive values
void Config::_store_methods_values(sLocation & location_struct, std::size_t values_end)
{
	std::vector<std::string>  values_vector;
	std::vector<std::string>::const_iterator it;

	location_struct.directives_in_conf_file["methods"] = true;
	this->_split_directive_values(values_end, values_vector);
	if (values_vector.size() > 3) {
		throw WrongValuesException();
	}
	location_struct.methods.get = false;
	location_struct.methods.post = false;
	location_struct.methods.del = false;
	for (it = values_vector.begin(); it != values_vector.end(); ++it) {
		if (*it == "get")
			location_struct.methods.get = true;
		else if (*it == "post")
			location_struct.methods.post = true;
		else if (*it == "delete")
			location_struct.methods.del = true;
		else
			throw WrongValuesException();
	}
}


//Store alias directive value
void Config::_store_alias_value(sLocation & location_struct, std::size_t value_end)
{
	location_struct.directives_in_conf_file["alias"] = true;
	std::string directive_value = this->_line.substr(this->_position, value_end - _position + 1);
	this->_cleanTrim(directive_value);
	if (directive_value[0] == '/')
		location_struct.alias = directive_value;
	else
		throw WrongValuesException();
}