#include "structures.hpp"

sRequest::sRequest() {
	this->size_expected = 0;
	this->parsed = false;
	this->is_cgi = false;
	this->complete = false;
	this->answered = false;
	this->body_expected = false;
	this->body_complete = false;
	this->chunked_complete = false;
	this->chunked_expected = false;
	this->c_hexpeeked = false;
	this->httpMessage = "";
	this->method = "";
	this->requestURL = "";
	this->path_file = "";
	this->fileName = "";
	this->extension = "";
	this->queryName = "";
	this->version = "";
	this->body = "";
	this->body_path = "";
	this->rootInLocation = "";
	this->rootPlusURL = "";
	this->temp_file = "";
}

sRequest::~sRequest() {}
