#include "StaticHandler.hpp"

S_Handler::S_Handler(sConnection& client) {
	if (!client.request || client.request->complete == false)
		return ;
	if (client.request->is_cgi == true || client.response->autoindex_response == true)
		return ;
	if (client.request->body_expected && !client.request->body_complete)
		return ;
	if (client.response->complete == true) {
		return;
	}

	std::cout << std::endl << GREEN_TEXT << "-STATIC-" << RESET_COLOR << std::endl;

	_path = client.request->rootPlusURL;
	if (_path[0] == '/') {
		_path = _path.substr(1);
	}
	populateMimeTypes();
	// std::cout << "Populating Response Structure..." << std::endl;
	client.response->file_name = client.request->fileName;
	client.response->file_path = client.request->rootPlusURL;
	stat(client.response->file_path.c_str(), &client.response->file_stat);
	try {
		handler(client);
	}
	catch(AError500Exception & e) {
		client.setStatusCode(500);
		std::cout << RED_TEXT << e.what() << RESET_COLOR << std::endl;
		GetMethod(client);
	}
}

S_Handler::~S_Handler() {}

std::string		deleteSpace(std::string str) {

	for (int i = 0; str[i]; i++) {
		if (str[i] == ' ') {
			str.erase(i, 1);
			i--;
		}
	}
	return str;
}

std::string		S_Handler::getFileExtension(std::string _path) {

	type = _path.substr(_path.find_last_of(".") + 1);
	return (type);
}

void			S_Handler::populateMimeTypes() {

	std::ifstream file;
	file.open("./configs/mime.types");
	std::string line;
	while(getline(file, line)) {
		if (!line.empty()) {
			line = deleteSpace(line);
			int separator_location = line.find('|');
			mimeTypes[line.substr(separator_location + 1, line.find(';') - (separator_location + 1))] = line.substr(0, separator_location);
		}
	}
}

void S_Handler::DeleteMethod(sConnection& connection)
{
	debug_printing(E_WARNING, (std::string) "Deleting file at -> " + connection.request->rootPlusURL.c_str());

	if (remove(connection.request->rootPlusURL.c_str()) != 0)
		connection.setStatusCode(403);
	else
	{
		connection.setStatusCode(200);
		connection.response->header = "HTTP/1.1 200 OK\r\n\r\n";
		
		std::string statusText = "OK";
		std::string responseContentType = "text/html";
		std::string file_path = "./www/html/delete_success.html";
		this->setResponseStruct(connection, responseContentType, statusText, file_path);
		
		connection.response->complete = true;
	}
	// throw S_HandlerException();
}

void		S_Handler::GetMethod(sConnection& client) {

	// std::cout << "HANDLING GET" << std::endl;
	if (client.response->error_flag == true) {
		client.request->headers["Connection"] = "close";
		if (client.getStatusCode() == 405) {
			if (client.request->method.empty() == false)
				client.response->file_path = "./www/" + client.request->bestLocation->error_pages[client.getStatusCode()];
			else
				client.response->file_path = "./www/errors/405.html";
		} else {
			client.response->file_path = "./www/" + client.request->bestLocation->error_pages[client.getStatusCode()];
		}
		if (access(client.response->file_path.c_str(), F_OK) == -1) {
			client.response->file_path = "./www/errors/" + intToString(client.getStatusCode()) + ".html";
		}
		stat(client.response->file_path.c_str(), &client.response->file_stat);
	}
	std::cout << BLUE_TEXT << client.response->file_path << RESET_COLOR << std::endl;
	print_size = intToString(client.response->file_stat.st_size);
	client.response->header = "HTTP/1.1 " + client._StatusString(client.getStatusCode()) + "\r\n";
	client.response->header += "Content-Length: " + print_size + "\r\n";
	client.response->header += "Content-Type: " + mimeTypes[getFileExtension(client.response->file_path)];
	if (client.request->fileName != "favicon.ico")
		client.response->header += "\r\nConnection: " + client.request->headers["Connection"];
	client.response->header += "\r\n\r\n";
	client.response->complete = true;
	return ;
}

// Set sResponse struct attributes to send a response (through sendResponse function) with a file as a body.
void		S_Handler::setResponseStruct(sConnection& connection, std::string & contentType, std::string & statusText, std::string & responseFilePath) {

	struct stat fileStat;

	if (access(responseFilePath.c_str(), F_OK) == -1 || access(responseFilePath.c_str(), R_OK) == -1)
		throw ResponseFileException();
	stat(responseFilePath.c_str(), &fileStat);
	connection.response->header = connection.request->version + " " + intToString(connection.getStatusCode()) + " " + statusText + "\r\n";
	connection.response->header += "Content-Length: " + intToString(fileStat.st_size) + "\r\n";
	connection.response->header += "Content-Type: " + contentType + "\r\n";
	connection.response->header += "Connection: " + connection.request->headers["Connection"] + "\r\n\r\n";
	connection.response->file_path = responseFilePath;
	connection.response->file_stat = fileStat;
	connection.response->complete = true;
}

// Check if the request was made by a form
bool	S_Handler::isForm(sConnection& connection) const {

	std::map<std::string, std::string>	headers = connection.request->headers;
	std::string							str = "multipart/form-data";

	return (headers.find("Content-Type") != headers.end() && headers["Content-Type"].substr(0, str.length()) == "multipart/form-data");
}

// Set file name with a default base name ("file_uploaded")
// and file extension based on Content-Type if possible, otherwise the extension will be "bin".
std::string	S_Handler::setDefaultFileName(std::map<std::string, std::string> & headers) {

	std::string	FileBaseName = "file_uploaded"; //default base name
	std::string	FileExtension = "bin"; //default extension
	std::string	newFileName = "";

	if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].empty() == false) {
		std::map<std::string, std::string>::const_iterator	it;

		for (it = this->mimeTypes.begin(); it != mimeTypes.end(); ++it) {
			if (it->second == headers["Content-Type"])
				FileExtension = it->first;
		}
	}
	newFileName = FileBaseName + "." + FileExtension;
	return newFileName;
}

// Manage the upload of a file whose name already exists.
// If the file already exists, it adds a number to the file name:
// if there are 2 identical files, the second one will have a "1" after its name;
// if there are 3 identical files, the third one will have a "2" after its name, and so on (only if the files are upoloaded in the same running of the program).
void		S_Handler::manageSameNameFiles(std::string & filePath, std::string & fileName, std::map<std::string, int> & FilesPaths) {

	if (isFileExisting(filePath) == false)
		FilesPaths.insert(std::pair<std::string, int>(filePath, 0));
	else while (isFileExisting(filePath) == true) {
		FilesPaths[filePath]++;
		addNumberToFileName(filePath, fileName, FilesPaths[filePath]);
	}
}

//Set body and file name of uploading file
void		S_Handler::setBodyAndFilename(sConnection& connection, std::string & body, std::string & fileName) {

	sRequest *	request = connection.request;

	if (isForm(connection) == true) { //form case
		// std::cout << BLUE_TEXT << "Case multipart/form-data" << RESET_COLOR << std::endl;
		if (request->forms.size() == 0)
			throw EmptyFormVectorException();
		body = request->forms[0]._content;
	}
	else
		body = request->body;
	if (isForm(connection) == true && request->forms[0]._filename.empty() == false) //form case with a filename
			fileName = request->forms[0]._filename;
	else
		fileName = setDefaultFileName(request->headers);
	// std::cout << "fileName: " << fileName << std::endl;
}

// Handle the response to a post method
void		S_Handler::postMethod(sConnection& connection) {

	std::cout << std::endl << GREEN_TEXT << "-POST METHOD-" << RESET_COLOR << std::endl;

	std::string							newFileName = "";
	std::string							newFilePath = "";
	static std::map<std::string, int>	FilesPaths; //map where the key is the file path and the value is the number of files with the same path -1 
	sRequest *							request = connection.request;
	std::map<std::string, std::string>	headers = connection.request->headers;
	std::string							body = "";
	std::string							responseFilePath = "./www/html/post_ok.html";
	std::string							statusText = "OK";
	
	setBodyAndFilename(connection, body, newFileName);

	newFilePath = "./" + vectorFoldersList(request->folders) + newFileName;

	manageSameNameFiles(newFilePath, newFileName, FilesPaths);
	
	if (request->chunked_complete == true) { //if the post request is in chunked mode, it moves the temp file
		// std::cout << MAGENTA_TEXT << "Case chunked mode" << RESET_COLOR << std::endl;
		if (request->temp_file.empty() == true)
			throw EmptyTempFileException();
		// std::cout << "request->temp_file: " << request->temp_file << std::endl;
		if (moveFile(request->temp_file, newFilePath) == false)
			throw CouldNotMoveFileException();
	}
	else { //if there is no chunk mode, it creates the file
		if (createNewFile(newFilePath, body) == false)
			throw CannotUploadFileException();
	}

	std::cout << BLUE_TEXT << "FILE UPLOADED" << RESET_COLOR << std::endl << std::endl;
	connection.setStatusCode(200);
	statusText = "OK";
	
	std::string responseContentType = "text/html";
	setResponseStruct(connection, responseContentType, statusText, responseFilePath);
}

///////CATIA END


// Risposta redirection
// HTTP/1.1 301 Moved Permanently
// Location: /index.html

void responseRedirection(sConnection& client, const std::string& reason) {
	
	client.response->header = client.request->version + " " + intToString(client.request->bestLocation->redirection.first) + " " + reason;
	client.response->header += "\r\nLocation: " + client.request->bestLocation->redirection.second + "\r\n";
	// std::cout << client.response->header << std::endl;
	client.response->complete = true;
	client.request->headers["Connection"] = "close";
}

// HTTP/1.1 200 OK
// Content-Type: image/x-icon
// Content-Length: [lunghezza del contenuto]

// [Contenuto dell'icona]

void		S_Handler::handler(sConnection& client) {

	// client.setStatusCode(301);
	debug_printing(E_INFO, "Debugging " + numToString(client.getStatusCode()));
	if (client.getStatusCode() == 200) {
		// std::cout << "Evaluating method: \"" << client.request->method <<"\"" << std::endl;
		if (client.request->method == "GET") {
			if (client.request->fileName.empty() == false)
				S_Handler::GetMethod(client);
			if (client.request->fileName.empty() == true && client.request->bestLocation->autoindex == false) {
				client.setStatusCode(403);
				return;
			}
			autoIndex(client);
			return ;
		}
		else if (client.request->method == "POST") {
			postMethod(client);
			return;
		}
		else if (client.request->method == "DELETE") {
			DeleteMethod(client);
		}
	}
	if (client.getStatusCode() == 300) {
		// std::cout << "Case 300" << std::endl;
		responseRedirection(client, "Multiple Choices");
		return;
	}
	else if (client.getStatusCode() == 301) {
		// std::cout << "Case 301" << std::endl;
		responseRedirection(client, "Moved Permanently");
		return;
	}
	else if (client.getStatusCode() == 302) {
		// std::cout << "Case 302" << std::endl;
		responseRedirection(client, "Found");
		return;
	}
	else if (client.getStatusCode() >= 400) {
		// std::cout << "--------------ErrorPage Case---------------------" << std::endl;
		client.response->error_flag = true;
		S_Handler::GetMethod(client);
		return ;
	}
}
