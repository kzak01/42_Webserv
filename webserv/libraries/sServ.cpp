#include "structures.hpp"

sServ::sServ() : /*domain_names(1, ""), */ip_address("0.0.0.0"), port("8080"), root("./www/html"), \
        client_max_body_size(ONE_MEGABYTE), autoindex(false), index("index.html") {

    this->error_pages[400] = "errors/400.html";
    this->error_pages[401] = "errors/401.html";
    this->error_pages[403] = "errors/403.html";
    this->error_pages[404] = "errors/404.html";
    this->error_pages[405] = "errors/405.html";
    this->error_pages[413] = "errors/413.html";
    this->error_pages[500] = "errors/500.html";
    this->error_pages[504] = "errors/504.html";
    this->error_pages[505] = "errors/505.html";

    // sLocation   root_location;
    // this->locations.push_back()

    directives_in_conf_file["server_name"] = false;
    directives_in_conf_file["listen"] = false;
    directives_in_conf_file["root"] = false;
    directives_in_conf_file["error_page"] = false;
    directives_in_conf_file["client_max_body_size"] = false;
    directives_in_conf_file["autoindex"] = false;
    directives_in_conf_file["index"] = false;
    directives_in_conf_file["return"] = false;
    directives_in_conf_file["location"] = false;
    directives_in_conf_file["cgi_enable"] = false;
}

//Print the directives that are in conf file inside the server block

void sServ::print() {

    std::cout << "-----DIRECTIVES-----" << std::endl;
    if (this->directives_in_conf_file["server_name"] == true)
        std::cout << "Domain names: " << vectorElementsList(domain_names) << std::endl;
    if (this->directives_in_conf_file["listen"] == true) {
        std::cout << "Ip address: " << ip_address << std::endl;
        std::cout << "Port: " << port << std::endl;
    }
    if (this->directives_in_conf_file["root"] == true)
        std::cout << "Root: " << root << std::endl;
    if (this->directives_in_conf_file["error_page"] == true)
        std::cout << "Error pages:" << std::endl << mapElementsList(error_pages, 0);
    if (this->directives_in_conf_file["client_max_body_size"] == true)
        std::cout << "Client max body size: " << client_max_body_size << std::endl;
    if (this->directives_in_conf_file["autoindex"] == true)
        std::cout << std::boolalpha << "Autoindex: " << autoindex << std::endl;
    if (this->directives_in_conf_file["index"] == true)
        std::cout << "Index: " << index << std::endl;
    if (this->directives_in_conf_file["return"] == true) {
        std::cout << "Redirection:" << std::endl << "    code: " << redirection.first << \
            ", new URL: " << redirection.second << std::endl;
    }
    if (this->directives_in_conf_file["cgi_enable"] == true)
        std::cout << "Cgi enable: " << vectorElementsList(cgi_enable) << std::endl;
    if (this->directives_in_conf_file["location"] == true) {
        for (size_t i = 0; i < locations.size() ; i++) {
            std::cout << std::endl << "LOCATION " << i + 1 << ":" << std::endl;
            locations[i].print();
        }
    }
    std::cout << "--------------------" << std::endl;
//    std::cout << "SockFd: " << sockFd << std::endl;
    std::cout << "Socket: " << _listening_socket.sock_fd << std::endl;
    std::vector<sServ>::iterator it;
	for (it = this->virtual_servers.begin(); it != virtual_servers.end(); ++it) {
        it->print_virtual();
    }
}

void sServ::print_virtual() {

    std::cout << "-----VIRTUAL SERVER DIRECTIVES-----" << std::endl;
    if (this->directives_in_conf_file["server_name"] == true)
        std::cout << "Domain names: " << vectorElementsList(domain_names) << std::endl;
    if (this->directives_in_conf_file["listen"] == true) {
        std::cout << "Ip address: " << ip_address << std::endl;
        std::cout << "Port: " << port << std::endl;
    }
    if (this->directives_in_conf_file["root"] == true)
        std::cout << "Root: " << root << std::endl;
    if (this->directives_in_conf_file["error_page"] == true)
        std::cout << "Error pages:" << std::endl << mapElementsList(error_pages, 0);
    if (this->directives_in_conf_file["client_max_body_size"] == true)
        std::cout << "Client max body size: " << client_max_body_size << std::endl;
    if (this->directives_in_conf_file["autoindex"] == true)
        std::cout << std::boolalpha << "Autoindex: " << autoindex << std::endl;
    if (this->directives_in_conf_file["index"] == true)
        std::cout << "Index: " << index << std::endl;
    if (this->directives_in_conf_file["return"] == true) {
        std::cout << "Redirection:" << std::endl << "    code: " << redirection.first << \
            ", new URL: " << redirection.second << std::endl;
    }
    if (this->directives_in_conf_file["cgi_enable"] == true)
        std::cout << "Cgi enable: " << vectorElementsList(cgi_enable) << std::endl;
    if (this->directives_in_conf_file["location"] == true) {
        for (size_t i = 0; i < locations.size() ; i++) {
            std::cout << std::endl << "LOCATION " << i + 1 << ":" << std::endl;
            locations[i].print();
        }
    }
    std::cout << "--------------------" << std::endl;
//    std::cout << "SockFd: " << sockFd << std::endl;
    std::cout << "Socket: " << _listening_socket.sock_fd << std::endl;
}

sServ::~sServ() {}

std::ostream & operator<<(std::ostream & o, sServ const & serv) {
    o << "-----DIRECTIVES-----" << std::endl;
    o << "Domain names: " << vectorElementsList(serv.domain_names) << std::endl;
    o << "Ip address: " << serv.ip_address << std::endl;
    o << "Port: " << serv.port << std::endl;
    o << "Root: " << serv.root << std::endl;
    o << "Error pages:" << std::endl << mapElementsList(serv.error_pages, 0);
    o << "Client max body size: " << serv.client_max_body_size << std::endl;
    o << std::boolalpha << "Autoindex: " << serv.autoindex << std::endl;
    o << "Index: " << serv.index << std::endl;
    o << "Redirection:" << std::endl << "    code: " << serv.redirection.first << \
            ", new URL: " << serv.redirection.second << std::endl;

    o << "Cgi enable: " << vectorElementsList(serv.cgi_enable) << std::endl;

    for (size_t i = 0; i < serv.locations.size() ; i++)
		std::cout << std::endl << "LOCATION " << i + 1 << ":" << std::endl << serv.locations[i];
    o << "--------------------" << std::endl;
//    o << "SockFd: " << serv.sockFd << std::endl;
    o << "Socket: " << serv._listening_socket.sock_fd << std::endl;
    std::vector<sServ>::const_iterator it;
	for (it = serv.virtual_servers.begin(); it != serv.virtual_servers.end(); ++it) {
        std::cout << *it << std::endl;
    } 

    return o;
}
