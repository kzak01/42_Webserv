#ifndef PARSEREQUEST_HPP
# define PARSEREQUEST_HPP

# include "../../libraries/structures.hpp"
# include "../../libraries/includes.hpp"

class ParseRequest {
	private:
		ParseRequest();
		ParseRequest(const ParseRequest& other);
		ParseRequest& operator=(const ParseRequest& other);

	public:
		ParseRequest(sConnection& client);
		~ParseRequest();

		void parseURL(sConnection& client);
		void splitURL(const std::string& url, sConnection& client);
		std::map<std::string, std::string> splitQueryString(const std::string& queryString);
		// bool isValidQueryString(const std::string& query, sConnection& client);

		bool checkLocation(sConnection& client);
		std::vector<sLocation>::iterator checkIfExistLocation(sConnection& client);
		bool checkMethod(sConnection& client);
		void getFullPath(sConnection& client);
		void aliasFound(sConnection& client);
		void indexFound(sConnection& client);
		void returnFound(sConnection& client);

		bool checkAccess(sConnection& client);
		bool checkPermission(const std::string& currentPath, sConnection& client);
		bool fileExists(const std::string& path);
		bool canReadFile(const std::string& path);
		bool canWriteFile(const std::string& path);

		std::string normalizePath(const std::string& input);
		void checkServer(sConnection& client);
};

#endif


class ParseException : public std::exception {
	public:
		virtual const char *what() const throw()
		{
			return ("ERROR: PARSE GENERIC ERROR");
		}
};
