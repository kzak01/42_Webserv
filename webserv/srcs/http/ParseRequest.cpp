#include "ParseRequest.hpp"

ParseRequest::ParseRequest(sConnection& client) {
	if (client.request->parsed == true) {
		return;
	}
	if (client.getStatusCode() >= 300)
		return;
	std::cout << GREEN_TEXT << "> ParseRequest" << RESET_COLOR << std::endl;
	try {
		if (client.request->version != "HTTP/1.1") {
			client.setStatusCode(505);
			throw ParseException();
		}
		checkServer(client);
		parseURL(client);
		if (checkLocation(client) == false)
			throw ParseException();
		if (checkAccess(client) == false)
			throw ParseException();
		splitURL(client.request->rootPlusURL, client);
		if (client.request->headers.find("Connection") == client.request->headers.end()) {
			client.request->headers["Connection"] = "keep-alive";
		}
		if (client.request->headers["Transfer-Encoding"] == "chunked")
			client.request->chunked_expected = true;
		if (client.request->headers.find("Content-Length") != client.request->headers.end()) {
			const std::string& contentLengthHeader = client.request->headers["Content-Length"];
			client.request->size_expected = atoi(contentLengthHeader.c_str());
		}
	} catch(const std::exception& e) {
		std::cout << RED_TEXT << e.what() << RESET_COLOR << std::endl;
	}
	client.request->parsed = true;
}

ParseRequest::~ParseRequest() {}

//-----------------------Split and Tokenize URL------------------------------------------------------

void ParseRequest::parseURL(sConnection& client) {
	// Estrazione dell'URL e della posizione del carattere '?'
	std::string url = client.request->requestURL;
	size_t queryPos = url.find("?");

	if (queryPos != std::string::npos) {
		// La parte prima del '?' è il percorso del file
		std::string filePath = url.substr(0, queryPos);
		client.request->path_file = filePath;
		// La parte dopo il '?' è la query string
		std::string queryString = url.substr(queryPos + 1);
		client.request->queryName = queryString;

		// Tokenizzazione del percorso del file e della stringa di query
		splitURL(filePath, client);
		client.request->queryMap = splitQueryString(queryString);
	} else {
		// Se non c'è il carattere '?', il percorso del file è l'URL completo
		client.request->path_file = url;
		splitURL(url, client);
	}
}

void ParseRequest::splitURL(const std::string& url, sConnection& client) {
	if (client.getStatusCode() >= 300)
		return;

	// Pulizia dell'URL rimuovendo il '/' iniziale o il './' iniziale
	std::string cleanedURL = url;
	if (cleanedURL[0] == '/')
		cleanedURL = cleanedURL.substr(1);
	else if (cleanedURL[0] == '.' && cleanedURL[1] == '/')
		cleanedURL = cleanedURL.substr(2);
	
	// Tokenizziamo l'URL utilizzando il carattere "/"
	std::vector<std::string> pathTokens;
	char* token = strtok(const_cast<char*>(cleanedURL.c_str()), "/");
	while (token != NULL) {
		pathTokens.push_back(token);
		token = strtok(NULL, "/");
	}

	if (pathTokens.empty() == true) {
		return;  // Considera la radice come valida
	}
	// Controlla se c'è un file e popola il vettore delle cartelle di conseguenza
	std::vector<std::string> folders;
	size_t vectorSize = 0;
	if (pathTokens.back().find(".") != std::string::npos) {
		vectorSize = pathTokens.size() - 1;
		const std::string& fileName = pathTokens.back();
		client.request->fileName = fileName;
		size_t pos = fileName.find_last_of('.');
		client.request->extension = fileName.substr(pos + 1);
	} else {
		vectorSize = pathTokens.size();
	}
	for (size_t i = 0; i < vectorSize; i++) {
		folders.push_back(pathTokens[i]);
	}

	client.request->folders = folders;
}

//------------------------Split and Tokenize query----------------------------------------------

std::map<std::string, std::string> ParseRequest::splitQueryString(const std::string& queryString) {
	std::map<std::string, std::string> keyValuePairs;
	std::vector<std::string> pairs;

	// Dividiamo la query string utilizzando il carattere '&'
	size_t startPos = 0;
	size_t endPos;
	while ((endPos = queryString.find('&', startPos)) != std::string::npos) {
		std::string pair = queryString.substr(startPos, endPos - startPos);
		pairs.push_back(pair);
		startPos = endPos + 1;
	}
	std::string lastPair = queryString.substr(startPos);
	pairs.push_back(lastPair);

	// Dividiamo ogni coppia chiave-valore utilizzando il carattere '='
	for (std::vector<std::string>::iterator it = pairs.begin(); it != pairs.end(); ++it) {
		const std::string& pair = *it;
		size_t separatorPos = pair.find('=');
		if (separatorPos != std::string::npos) {
			std::string key = pair.substr(0, separatorPos);
			std::string value = pair.substr(separatorPos + 1);
			keyValuePairs[key] = value;
		} else
			keyValuePairs[pair] = pair;
	}
	return keyValuePairs;
}

// bool ParseRequest::isValidQueryString(const std::string& query, sConnection& client) {1
// 	(void)query;
// 	(void)client;
// 	//! non so cosa devo controllare
// 	return true;
// }

// ---------------------Varius check with Location------------------------------------------------------------------

bool ParseRequest::checkLocation(sConnection& client) {
	// Trova la miglior corrispondenza della posizione per la richiesta
	client.request->bestLocation = checkIfExistLocation(client);
	// std::cout << CYAN_TEXT << "Best Location Match: " << client.request->bestLocation->loc_path 
		// << "\nRoot: " << client.request->bestLocation->root << "\nIndex: " << client.request->bestLocation->index << RESET_COLOR << std::endl;
	// Verifica se il metodo é permesso
	if (checkMethod(client) == false)
		return false;
	// Ottieni il percorso completo considerando alias, root, e index
	getFullPath(client);
	return true;
}

std::vector<sLocation>::iterator ParseRequest::checkIfExistLocation(sConnection& client) {
	std::vector<sLocation>::iterator bestMatch;
	size_t bestMatchLenght = 0;
	// Itera attraverso le posizioni definite nel server
	for (std::vector<sLocation>::iterator it = client.server->locations.begin(); it != client.server->locations.end(); it++) {
		if (client.request->path_file.find(it->loc_path) == 0 && it->loc_path.length() > bestMatchLenght) {
			bestMatch = it;
			bestMatchLenght = it->loc_path.length();
		}
	}
	return bestMatch;
}

bool ParseRequest::checkMethod(sConnection& client) {
	// Verifica se il metodo HTTP specificato è consentito per la Location
	if (client.request->method == "POST")
		client.request->body_expected = true;
	if (client.request->bestLocation->directives_in_conf_file["methods"] == false)
		return true;
	if ((client.request->method == "GET" && client.request->bestLocation->methods.get == false)
			|| (client.request->method == "POST" && client.request->bestLocation->methods.post == false)
			|| (client.request->method == "DELETE" && client.request->bestLocation->methods.del == false)) {
				std::cout << RED_TEXT << "Error: Wrong Method" << RESET_COLOR << std::endl;
				client.setStatusCode(405);
				return false;
			}
	return true;
}

void ParseRequest::getFullPath(sConnection& client) {
	if (client.request->bestLocation->directives_in_conf_file["return"] == true) {
		returnFound(client);
		return;
	}
	// Gestisce l'alias, la root, e l'index per ottenere il percorso completo

	if (client.request->bestLocation->alias != "") {
		aliasFound(client);
	} else {
		if (client.request->bestLocation->root[client.request->bestLocation->root.size() - 1] == '/'){
			client.request->rootInLocation = client.request->bestLocation->root.substr(0, client.request->bestLocation->root.size() - 1);
		}
		else {
			client.request->rootInLocation = client.request->bestLocation->root;
		}
		client.request->rootPlusURL = client.request->rootInLocation + client.request->path_file;
	}
	client.request->rootPlusURL = normalizePath(client.request->rootPlusURL);
	// Se non è specificato un nome di file e l'index è definito, trova l'index
	if (client.request->fileName.empty() == true && client.request->bestLocation->index != "") {
		indexFound(client);
	}
	if (client.request->rootPlusURL[0] != '.' && client.request->rootPlusURL[1] != '/')
		client.request->rootPlusURL = "./" + client.request->rootPlusURL;
	else if (client.request->rootPlusURL[0] != '.' && client.request->rootPlusURL[1] == '/')
		client.request->rootPlusURL = "." + client.request->rootPlusURL;
	client.request->rootPlusURL = normalizePath(client.request->rootPlusURL);
}

// ----------------------Varius directives like Alias, Index and Return-------------------------------------------------------------------------

void ParseRequest::aliasFound(sConnection& client) {
	// Gestisce l'alias nella richiesta
	std::string str = client.request->path_file;
	std::string from = client.request->bestLocation->loc_path;
	std::string to = client.request->bestLocation->alias;
	size_t start_pos = str.find(from);
	bool haveSlash = !from.empty() && from[from.size() - 1] == '/';

	// Se l'alias contiene un'estensione, imposta il nome del file e l'estensione
	if (to.find(".") != std::string::npos) {
		client.request->fileName = to;
		size_t pos = to.find_last_of('.');
		client.request->extension = to.substr(pos + 1);
	}
	// std::cout << BOLD_RED_TEXT << "Alias directive was found!!!\n" << "str: " << str << "\nfrom: " << from << "\nto: " << to << RESET_COLOR << std::endl;
	// Sostituisci l'alias nel percorso
	if (start_pos != std::string::npos)
		str.replace(start_pos, from.length(), to);
	client.request->rootInLocation = client.request->bestLocation->root;
	client.request->rootPlusURL = client.request->rootInLocation + str;
	if (haveSlash == true)
		client.request->rootPlusURL = client.request->rootPlusURL + "/";
	// std::cout << BOLD_RED_TEXT << "new path from Alias: " << client.request->rootPlusURL << RESET_COLOR << std::endl;
}

void ParseRequest::indexFound(sConnection& client) {
	// std::cout << BOLD_MAGENTA_TEXT << "Index directive was found!!!\nIndex: " << client.request->bestLocation->index << RESET_COLOR << std::endl;
	
	std::vector<std::string> indexTokens;
	std::string index = client.request->bestLocation->index;
	char* token = strtok(const_cast<char*>(index.c_str()), " ");
	while (token != NULL) {
		indexTokens.push_back(token);
		token = strtok(NULL, " ");
	}
	for (std::vector<std::string>::iterator it = indexTokens.begin(); it != indexTokens.end(); it++) {
		std::string currentPath;
		if (client.request->rootPlusURL[client.request->rootPlusURL.size() - 1] != '/')
			currentPath = client.request->rootPlusURL + "/" + *it;
		else
			currentPath = client.request->rootPlusURL + *it;
		// std::cout << BOLD_MAGENTA_TEXT << "watching: " << currentPath << RESET_COLOR << std::endl;
		if (fileExists(currentPath) == true) {
			// std::cout << BOLD_MAGENTA_TEXT << "Token that exist: " << *it << RESET_COLOR << std::endl;
			client.request->rootPlusURL = client.request->rootPlusURL + "/" + *it;
			std::string checkIt = *it;
			if (checkIt.find('.') != std::string::npos) {
				client.request->fileName = checkIt;
				size_t pos = checkIt.find_last_of('.');
				client.request->extension = checkIt.substr(pos + 1);
				return;
			}
		}
	}
}

void ParseRequest::returnFound(sConnection& client) {
	int errorCode = client.request->bestLocation->redirection.first;
	client.setStatusCode(errorCode);
	return;
}

// ----------------------Check with Access-------------------------------------------------------------------------

bool ParseRequest::checkAccess(sConnection& client) {
	// std::cout << BOLD_GREEN_TEXT << "CheckAccess: " << client.request->rootPlusURL << RESET_COLOR << std::endl;
	if (client.getStatusCode() >= 300)
		return true;
	// std::string cleanedURL = client.request->rootPlusURL;
	// if (cleanedURL[0] == '/')
	// 	cleanedURL = cleanedURL.substr(1);
	// else if (cleanedURL[0] == '.' && cleanedURL[1] == '/')
	// 	cleanedURL = cleanedURL.substr(2);
	// std::vector<std::string> pathTokens;
	// char* token = strtok(const_cast<char*>(cleanedURL.c_str()), "/");
	// while (token != NULL) {
	// 	pathTokens.push_back(token);
	// 	token = strtok(NULL, "/");
	// }
	// int first = 0;
	// std::string currentPath;
	// for (std::vector<std::string>::iterator it = pathTokens.begin(); it != pathTokens.end(); it++) {
	// 	if (first == 0) {
	// 		first++;
	// 		currentPath += "./" + *it;
	// 	} else {
	// 		currentPath += "/" + *it;
	// 	}
	// 	std::cout << BOLD_GREEN_TEXT << "CurrentPath: " << currentPath << RESET_COLOR << std::endl;
	// 	if (checkPermission(currentPath, client) == false) {
	// 		std::cout << RED_TEXT << "Error: This path have problem: " << currentPath << RESET_COLOR << std::endl;
	// 		return false;
	// 	}
	// }
	if (checkPermission(client.request->rootPlusURL, client) == false) {
		std::cout << RED_TEXT << "Error: This path have problem: " << client.request->rootPlusURL << RESET_COLOR << std::endl;
		return false;
	}
	return true;
}

bool ParseRequest::checkPermission(const std::string& currentPath, sConnection& client) {
	if (fileExists(currentPath) == false) {
		// std::cout << "!fileExist" << std::endl;
		client.setStatusCode(404); // Not Found
		return false;
	}
	if (client.request->method == "GET") {
		if (canReadFile(currentPath) == false) {
			// std::cout << "!fileRead" << std::endl;
			client.setStatusCode(403); // Forbidden
			return false;
		}
	}
	if (client.request->method == "POST" || client.request->method == "DELETE") {
		if (canWriteFile(currentPath) == false) {
			// std::cout << "fileWrite" << std::endl;
			client.setStatusCode(403); // Forbidden
			return false;
		}
	}
	return true;
}

bool ParseRequest::fileExists(const std::string& path) {
	return access(path.c_str(), F_OK) == 0;
}

bool ParseRequest::canReadFile(const std::string& path) {
	return access(path.c_str(), R_OK) == 0;
}

bool ParseRequest::canWriteFile(const std::string& path) {
	return access(path.c_str(), W_OK) == 0;
}

// ----------------------Utils-------------------------------------------------------------------------

std::string ParseRequest::normalizePath(const std::string& input) {
	// Normalizza il percorso rimuovendo gli slash duplicati consecutivi
	std::string result;
	result.reserve(input.size());

	// Flag per tracciare se abbiamo già incontrato uno slash
	bool encounteredSlash = false;

	for (std::string::size_type i = 0; i < input.size(); ++i) {
		char c = input[i];
		if (c == '/') {
			// Se è uno slash, e non ne abbiamo ancora incontrato uno,
			// aggiungiamo uno slash al risultato e impostiamo il flag su true.
			if (!encounteredSlash) {
				result.push_back(c);
				encounteredSlash = true;
			}
		} else {
			// Se non è uno slash, aggiungiamolo al risultato e
			// reimpostiamo il flag su false.
			result.push_back(c);
			encounteredSlash = false;
		}
	}
	return result;
}

void ParseRequest::checkServer(sConnection& client) {
	if (client.request->headers.find("Host") != client.request->headers.end()) {
		std::cout << RED_TEXT << client.request->headers["Host"] << RESET_COLOR << std::endl;
		for (std::vector<std::string>::iterator it = client.server->domain_names.begin(); it != client.server->domain_names.end(); it++) {
			if (client.request->headers["Host"] == *it)
				return;
		}
		for (std::vector<sServ>::iterator it = client.server->virtual_servers.begin(); it != client.server->virtual_servers.end(); it++) {
			for (std::vector<std::string>::iterator vt = it->domain_names.begin(); vt != it->domain_names.end(); vt++) {
				if (client.request->headers["Host"] == *vt) {
					client.server = &(*it);
					return;
				}
			}
		}
	}
}
