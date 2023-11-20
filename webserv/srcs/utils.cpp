#include "utils.hpp"

//Convert a string to an int
int	ft_stoi(std::string const & str) {
	
	int	i;
	int	num;

	num = 0;
	i = 0;
	while (str[i] >= 48 && str[i] <= 57)
	{
		num = num * 10 + (str[i] - 48);
		i++;
	}
	return (num);
}

std::string	intToString(int	n) {

	std::stringstream tmp;
	std::string str;
	tmp << n;
	tmp >> str;
	return str;
}

//Return a string with all the elements of a vector of strings separated by a space
std::string vectorElementsList(const std::vector<std::string> & vec) {

    std::string str = "";
	std::vector<std::string>::const_iterator it;

	for (it = vec.begin(); it != vec.end(); ++it) {
        str += *it + " ";
    }
    return str;
}

//Return a string with all the elements of a map<int, string> (for error pages)
std::string mapElementsList(const std::map<int, std::string> & map, bool indent) {

    std::string str = "";
    std::string key_str = "";
	std::map<int, std::string>::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it) {
        std::stringstream ss;
        ss << it->first;
        key_str = ss.str();
		if (indent == 1)
			str += "    ";
        str += "    code: " + key_str + ", page: " + it->second + "\n";
	}
    return str;
}

//check if a file exists
bool	isFileExisting(const std::string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
}

//Return a string with all the elements of a vector of strings,
//separated by and ending with "/" (usefull for folders)
std::string vectorFoldersList(const std::vector<std::string> & folders) {

    std::string str = "";
	std::vector<std::string>::const_iterator it;

	for (it = folders.begin(); it != folders.end(); ++it) {
        str += *it + "/";
    }
    return str;
}

//add a number to the name of the file
void	addNumberToFileName(std::string & filePath, std::string & fileName, int num_to_add) {

	std::ostringstream		ss;
	std::size_t				dotPos;
	std::string				baseName;
	std::string				extension;

	dotPos = fileName.find('.');
	baseName = fileName.substr(0, dotPos);
	extension = fileName.substr(dotPos + 1, fileName.length() - dotPos + 1);
	ss << num_to_add;
	filePath.replace(filePath.length() - extension.length() - 1 - baseName.length(), baseName.length(), baseName + "_" + ss.str());
	fileName.replace(0, dotPos, baseName + "_" + ss.str());
}

//Move a file from sourcePath to destinationPath. SourcePath and destinationPath are path that include file name.
bool	moveFile(const std::string& sourcePath, const std::string& destinationPath) {

    std::cout << std::endl << GREEN_TEXT << "In moveFile" << RESET_COLOR << std::endl;

	std::ifstream source(sourcePath.c_str(), std::ios::binary);
    std::ofstream destination(destinationPath.c_str(), std::ios::binary);
	
	if (!source)
		std::cout << "!source" << std::endl;
	if (!destination)
		std::cout << "!destination" << std::endl;
    if (!source || !destination) {
        return false;
    }

    destination << source.rdbuf();

    source.close();
    destination.close();

	// Check if all is ok until now
    if (destination.fail() || source.fail()) {
		std::cout << "destination.fail() || source.fail()" << std::endl;
        return false;
    }

    // Delete original file
    if (remove(sourcePath.c_str()) != 0) {
		std::cout << "remove(sourcePath.c_str()" << std::endl;
        return false;
    }

    return true;
}

bool		createNewFile(std::string & filePath, std::string & content) {

	// std::cout << "filePath: " << filePath << std::endl;
	
	std::ofstream newFile(filePath.c_str());
	if (!newFile.is_open())
		return false;
	newFile << content; //write the body in the file
	newFile.close();
	return true;
}
