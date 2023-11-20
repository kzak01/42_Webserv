#ifndef SANITIZE_HPP
# define SANITIZE_HPP
// # include "includes.hpp"
// # include "types.hpp"
// # include "templates.tpp"

#define SANITIZE_STRING(str, output_string) \
	output_string.clear(); \
    for (size_t i = 0; i < (str).length(); ++i) { \
        char c = (str)[i]; \
        if (std::isprint(c)) { \
            (output_string) += c; \
        } \
    }

#define SANITIZE_STREAM(input_stream, output_string) \
    char c; \
	output_string.clear(); \
    while ((input_stream).get(c)) { \
        if (std::isprint(c)) { \
            (output_string) += c; \
        } \
    }

// void safeString(const std::string& input, std::string& output) {
// 	output.clear();
//     for (size_t i = 0; i < input.length(); ++i) {
//         char c = input[i];
//         if (std::isprint(c)) {
//             output += c;
//         }
//     }
// }

// void safeString(std::istream& input_stream, std::string& output) {
// 	output.clear();
//     char c;
//     while (input_stream.get(c)) {
//         if (std::isprint(c)) {
//             output += c;
//         }
//     }
// }

#endif