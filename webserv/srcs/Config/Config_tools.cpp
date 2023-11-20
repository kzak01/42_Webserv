#include "../../libraries/includes.hpp"
#include "Config.hpp"

// Check if the directive in conf file is valid
bool Config::_is_keywords(std::string word, std::vector<std::string> keywords) const {

	std::vector<std::string>::const_iterator it;

	for (it = keywords.begin(); it != keywords.end(); ++it) {
		if (word == *it)
			return true;
	}
	return false;
}

// Delete spaces at the beginning and at the and of a string
void Config::_cleanTrim(std::string & str)
{
    str.erase(0, str.find_first_not_of(" "));
    while (!str.empty() && str[str.length() - 1] == ' ')
        str.erase(str.length() - 1);
}

// Checks if a line in config file contains just spaces and comments
bool Config::_is_anything_inside_line() {
	this->_position = this->_line.find_first_not_of(' ', _position);
	return (!(_position == std::string::npos || _line[_position] == '#'));
}

// Checks if there is the word "word" in the current position in the config file
bool Config::_is_that_word_now(std::string word) const {

    std::size_t		word_pos = std::string::npos;

    word_pos = _line.find(word, this-> _position);
	return (word_pos == _position);
}

bool	Config::_are_all_servernames_different(std::vector<std::string> domain_names_1, std::vector<std::string> domain_names_2) {

	std::cout << MAGENTA_TEXT << "_are_all_servernames_different" << RESET_COLOR << std::endl;
	std::vector<std::string>::const_iterator	it_1;
	std::vector<std::string>::const_iterator	it_2;
	//bool										found_same_name;
	
	// if (domain_names_1.size() != domain_names_2.size())
	// 	return true;
	if (domain_names_1.size() == 0 && domain_names_2.size() == 0) {
		std::cout << "domain_names_1.size() == 0 && domain_names_2.size() == 0" << std::endl;
		//throw ServersConflictException();
		return false;
	}
	else if (domain_names_2.size() == 0) {
		//std::cout << "domain_names_2.size() == 0" << std::endl;
		return false;
	}
	else if (domain_names_1.size() == 0) {
		std::cout << "domain_names_1.size() == 0" << std::endl;
		return true;
	}
	if (domain_names_1 == domain_names_2) {
		std::cout << "domain_names_1 == domain_names_2" << std::endl;
		//throw ServersConflictException();
		return false;
	}
	for (it_1 = domain_names_1.begin(); it_1 != domain_names_1.end(); ++it_1) {
		//found_same_name = false;
		for (it_2 = domain_names_2.begin(); it_2 != domain_names_2.end(); ++it_2) {
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

	// for (it_1 = domain_names_2.begin(); it_1 != domain_names_2.end(); ++it_1) {
	// 	found_same_name = false;
	// 	for (it_2 = domain_names_1.begin(); it_2 != domain_names_1.end(); ++it_2) {
	// 		if (*it_1 == *it_2) {
	// 			found_same_name = true;
	// 			break;
	// 		}
	// 	}
	// 	if (found_same_name == false)
	// 		return true;
	// }

	std::cout << MAGENTA_TEXT << "_are_all_servernames_different END" << RESET_COLOR << std::endl;
	return true;	
}

sServ * Config::_find_parent_server(sServ & serv_struct) {

	// std::cout << BLUE_TEXT << "_find_parent_server" << RESET_COLOR << std::endl;
	std::vector<sServ>::iterator	it;
	sServ *							server_pointer = NULL;

	for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
		if (it->ip_address == serv_struct.ip_address && it->port == serv_struct.port) {
			server_pointer = &(*it);
			return server_pointer;
		}
	}
	return NULL;

	// for (it = this->_servers.begin(); it != this->_servers.end(); ++it) {
	// 	if (it->ip_address == serv_struct.ip_address && it->port == serv_struct.port && _are_all_servernames_different(it->domain_names, serv_struct.domain_names) == true) {
	// 		std::cout << MAGENTA_TEXT << "virtual server" << RESET_COLOR << std::endl;
	// 		server_pointer = &(*it);
	// 		//return server_pointer;
	// 	}
	// }
	return server_pointer;
}

