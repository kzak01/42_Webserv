#include "../../libraries/includes.hpp"
# include "Config.hpp" 

template<typename T>
//Store root and index directives values (template function)
void	Config::_store_simple_directive_value(T & structure, std::size_t value_end, std::string directive) {
    
    std::string directive_value = this->_line.substr(this->_position, value_end - _position + 1);

    this->_cleanTrim(directive_value);
    if (directive == "root") {
        structure.root = directive_value;
        structure.directives_in_conf_file["root"] = true;
    }
    // else if (directive == "client_max_body_size") {
    //     structure.client_max_body_size = directive_value;
    //     structure.directives_in_conf_file["client_max_body_size"] = true;
    // }
    else if (directive == "index") {
        structure.index = directive_value;
        structure.directives_in_conf_file["index"] = true;
    }
}

template<typename T>
//Store error_page directive values (template function)
void	Config::_store_error_page_values(T & structure, std::size_t values_end) {

    std::vector<std::string> 	values_vector;
    int							status_code;

    structure.directives_in_conf_file["error_page"] = true;
    this->_split_directive_values(values_end, values_vector);
    if (values_vector.size() != 2) {
        throw WrongValuesException();
    }
    if (values_vector[0].length() != 3) {
        _position = _line.find(values_vector[0]);
        throw WrongValuesException();
    }
    status_code = atoi(values_vector[0].c_str());
    if (structure.error_pages.find(status_code) != structure.error_pages.end())
        structure.error_pages[status_code] = values_vector[1];
    else {
        _position = _line.find(values_vector[0]);
        throw WrongValuesException();
    }
}

template<typename T>
//Store client_max_body_size directive value (template function)
void	Config::_store_client_max_body_size_value(T & structure, std::size_t value_end) {

    std::string directive_value = this->_line.substr(this->_position, value_end - _position + 1);
    char        unit;
    int         multiplier = 1;
    int         num_length = 1;
    long        num;
    char        last_char;
    char        *end_ptr;
    std::size_t max_body_size;

    structure.directives_in_conf_file["client_max_body_size"] = true;
    this->_cleanTrim(directive_value);
    if (directive_value.empty() == true)
        throw WrongValuesException();
    last_char = directive_value[directive_value.length() - 1];
    unit = tolower(last_char);
    switch (unit) {
        case 'k':
            multiplier = 1024;
            break;
        case 'm':
            multiplier = 1024 * 1024;
            break;
        default:
            if (std::isdigit(unit) == false)
                throw WrongValuesException();
    }
    // If a unit was found, it will be excluded from the numeric conversion
    num_length = std::isdigit(unit) ? directive_value.length() : directive_value.length() - 1;
    num = std::strtol(directive_value.substr(0, num_length).c_str(), &end_ptr, 10);
    if (*end_ptr != '\0' || num < 0)
        throw WrongValuesException();
    max_body_size = static_cast<std::size_t>(num) * multiplier;
    if (max_body_size > MAX_BODY_SIZE)
        throw WrongValuesException();
    structure.client_max_body_size = max_body_size;
}

template<typename T>
//Store autoindex directive value (template function)
void	Config::_store_autoindex_value(T & structure, std::size_t value_end) {
    
    std::string directive_value = this->_line.substr(this->_position, value_end - _position + 1);

    structure.directives_in_conf_file["autoindex"] = true;
    this->_cleanTrim(directive_value);
    if (directive_value == "on")
        structure.autoindex = true;
    else if (directive_value == "off")
        structure.autoindex = false;
    else
        throw WrongValuesException();
}

template<typename T>
//Store return directive values (template function)
void	Config::_store_return_values(T & structure, std::size_t values_end) {
    
    std::vector<std::string> 	values_vector;
    int							status_code;

    structure.directives_in_conf_file["return"] = true;
    this->_split_directive_values(values_end, values_vector);
    if (values_vector.size() != 2)
        throw WrongValuesException();
    if (values_vector[0].length() != 3) {
        _position = _line.find(values_vector[0]);
        throw WrongValuesException();
    }
    status_code = atoi(values_vector[0].c_str());
    if (status_code < 300 || status_code > 308) {
        _position = _line.find(values_vector[0]);
        throw WrongValuesException();
    }
    structure.redirection.first = status_code;
    structure.redirection.second = values_vector[1];
}
