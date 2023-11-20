#include "structures.hpp"

sHttpMethods::sHttpMethods() : get(true), post(true), del(true) {

}

sHttpMethods::~sHttpMethods() {

}

sHttpMethods::sHttpMethods(sHttpMethods const & other) {

    *this = other;
}

sHttpMethods & sHttpMethods::operator=(sHttpMethods const & other) {

    this->get = other.get;
    this->post = other.post;
    this->del = other.del;
        
    return *this;
}

std::ostream & operator<<(std::ostream & o, sHttpMethods const & http_methods) {

    if (http_methods.get == true)
        o << "get ";
    if (http_methods.post == true)
        o << "post ";
    if (http_methods.del == true)
        o << "delete ";
    o << std::endl;
        
    return o;
}
