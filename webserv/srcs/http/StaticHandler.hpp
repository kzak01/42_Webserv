/* ************************************************************************** */
/*																			*/
/*														:::	  ::::::::   */
/*   S_Handler.hpp									   :+:	  :+:	:+:   */
/*													+:+ +:+		 +:+	 */
/*   By: catia <catia@student.42.fr>				+#+  +:+	   +#+		*/
/*												+#+#+#+#+#+   +#+		   */
/*   Created: 2023/09/08 11:40:02 by crossi			#+#	#+#			 */
/*   Updated: 2023/10/18 18:20:41 by catia			###   ########.fr	   */
/*																			*/
/* ************************************************************************** */

#ifndef STATICHANDLER_HPP
# define STATICHANDLER_HPP

#include <fstream>
#include "sys/stat.h"
#include <iostream>
#include <map>
#include <sstream>
#include "../../libraries/structures.hpp"
#include "../../libraries/includes.hpp"
#include "../../libraries/classes.hpp"
#include "../utils.hpp"

class S_Handler
{
private:
	/* data */
	//int fd_error;
	int 								fd;
	int 								fsize;
	int									len; 
	std::string 						_path;
	std::string 						absolute_path;
	std::string 						type;
	std::string 						header;
	std::string 						print_size;
	// Da mettere nel server
	std::map<std::string, std::string>	mimeTypes;

	//CATIA
	void		postMethod(sConnection& connection);
	std::string	setDefaultFileName(std::map<std::string, std::string> & headers);
	void		manageSameNameFiles(std::string & filePath, std::string & fileName, std::map<std::string, int> & FilesPaths);
	void		setBodyAndFilename(sConnection& connection, std::string & body, std::string & fileName);

	S_Handler();

public:
	S_Handler(sConnection& client);
	~S_Handler();

	std::string	getFileExtension(std::string _path);
	// void		S_Handler::sendError(sConnection& client);
	void		GetMethod(sConnection& client);
	void		handler(sConnection& client);
	// Da mettere nel server
	void		populateMimeTypes();

	//TOMMASO
	void		DeleteMethod(sConnection& connection);

	//CATIA
	void		setResponseStruct(sConnection& connection, std::string & contentType, std::string & statusText, std::string & responseFilePath);
	bool		isForm(sConnection& connection) const;

	//Kevin
	// void redirectionReturn(sConnection& client);
	// void errorPage(sConnection& client);
	// void responseError(sConnection& client, const std::string& reason, int errorCase);
	// void faviconResponse(sConnection& client);


	class S_HandlerException : public std::exception {
		public:
			virtual const char *what() const throw() {
				return ("WTF!");
			}
	};

	//CATIA
	class AError500Exception : public std::exception {
		public:
			virtual const char *what() const throw() {
				return "Error: 500";
			}
	};
	
	//CATIA
	class CannotUploadFileException : public AError500Exception {
		public:
			virtual const char *what() const throw() {
				return "Error: cannot upload file";
			}
	};

	//CATIA
	class ResponseFileException : public AError500Exception {
		public:
			virtual const char *what() const throw() {
				return "Error: the response file does not exist or cannot be opened";
			}
	};

	//CATIA
	class EmptyFormVectorException : public AError500Exception {
		public:
			virtual const char *what() const throw() {
				return "Error: request->forms vector is empty";
			}
	};

	//CATIA
	class EmptyTempFileException : public AError500Exception {
		public:
			virtual const char *what() const throw() {
				return "Error: request->temp_file is empty";
			}
	};

	//CATIA
	class CouldNotMoveFileException : public AError500Exception {
		public:
			virtual const char *what() const throw() {
				return "Error: could not move request->temp_file";
			}
	};
};

#endif
