#ifndef UTILS_HPP
# define UTILS_HPP

#include "../libraries/includes.hpp"
#include <fstream>
#include "sys/stat.h"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>



int	        ft_stoi(std::string const & str);

std::string vectorElementsList(const std::vector<std::string> & vec);
std::string mapElementsList(const std::map<int, std::string> & map, bool indent);

bool		isFileExisting(const std::string& filePath);
std::string vectorFoldersList(const std::vector<std::string> & folders);
void		addNumberToFileName(std::string & filePath, std::string & fileName, int num_to_add);
bool		moveFile(const std::string& sourcePath, const std::string& destinationPath);
bool		createNewFile(std::string & filePath, std::string & content);

//Trasformazione di interi a stringhe fatta con gli sstream
std::string	intToString(int	n);

template <typename T>
std::string	numToString(T number)
{
	std::stringstream	strstream;
	std::string 		strnumber;

	strstream << number;
	strstream >> strnumber;
	return (strnumber);
}

#define HEX_TO_INT(hexStr, intValue) do { \
    std::stringstream hexConverter; \
    hexConverter << std::hex << hexStr; \
    hexConverter >> intValue; \
} while(0)

#endif