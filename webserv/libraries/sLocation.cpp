#include "structures.hpp"

sLocation::sLocation() : loc_path(""), root("./www/html"), client_max_body_size(ONE_MEGABYTE), \
         autoindex(false), index("index.html"), alias(""), redirection(0, "") {
    this->error_pages[400] = "errors/400.html";
    this->error_pages[401] = "errors/401.html";
    this->error_pages[403] = "errors/403.html";
    this->error_pages[404] = "errors/404.html";
    this->error_pages[405] = "errors/405.html";
    this->error_pages[413] = "errors/413.html";
    this->error_pages[500] = "errors/500.html";
    this->error_pages[504] = "errors/504.html";
    this->error_pages[505] = "errors/505.html";

    directives_in_conf_file["root"] = false;
    directives_in_conf_file["error_page"] = false;
    directives_in_conf_file["client_max_body_size"] = false;
    directives_in_conf_file["methods"] = false;
    directives_in_conf_file["autoindex"] = false;
    directives_in_conf_file["index"] = false;
    directives_in_conf_file["alias"] = false;
    directives_in_conf_file["return"] = false;
}

sLocation::sLocation(sLocation const & other) {
    *this = other;
}

sLocation & sLocation::operator=(sLocation const & other) {
    this->loc_path = other.loc_path;
    this->root = other.root;
    this->error_pages = other.error_pages;
    this->client_max_body_size = other.client_max_body_size;
    this->methods = other.methods;
    this->autoindex = other.autoindex;
    this->index = other.index;
    this->alias = other.alias;
    this->redirection = other.redirection;

    this->directives_in_conf_file = other.directives_in_conf_file;

    return *this;
}

//Print the directives that are in conf file inside the location block
void sLocation::print() {
    std::cout << "  location path: " << loc_path << std::endl;

    if (this->directives_in_conf_file["root"] == true)
        std::cout << "  root: " << root << std::endl;
    if (this->directives_in_conf_file["error_page"] == true)
        std::cout << "  error pages:" << std::endl << mapElementsList(error_pages, 1);
    if (this->directives_in_conf_file["client_max_body_size"] == true)
        std::cout << "  client max body size: " << client_max_body_size << std::endl;
    if (this->directives_in_conf_file["methods"] == true)
        std::cout << "  HTTP methods: " << methods;
    if (this->directives_in_conf_file["autoindex"] == true)
        std::cout << std::boolalpha << "  autoindex: " << autoindex << std::endl;
    if (this->directives_in_conf_file["index"] == true)
        std::cout << "  index: " << index << std::endl;
    if (this->directives_in_conf_file["alias"] == true)
        std::cout << "  alias: " << alias << std::endl;
    if (this->directives_in_conf_file["return"] == true) {
        std::cout << "  redirection:" << std::endl << "   code: " << redirection.first << \
            ", new URL: " << redirection.second << std::endl;
    } 
}

sLocation::~sLocation() {}

std::ostream & operator<<(std::ostream & o, sLocation const & location) {
    o << "  location path: " << location.loc_path << std::endl;
    o << "  root: " << location.root << std::endl;
    o << "  error pages:" << std::endl << mapElementsList(location.error_pages, 1);
    o << "  client max body size: " << location.client_max_body_size << std::endl;
    o << "  HTTP methods: " << location.methods;
    o << std::boolalpha << "    autoindex: " << location.autoindex << std::endl;
    o << "  index: " << location.index << std::endl;
    o << "  alias: " << location.alias << std::endl;
    o << "  redirection:" << std::endl << "   code: " << location.redirection.first << \
            ", new URL: " << location.redirection.second << std::endl;

    return o;
}
