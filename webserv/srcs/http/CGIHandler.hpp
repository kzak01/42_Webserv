#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "../../libraries/includes.hpp"
# include <iostream>
# include <fstream> 

# define CGI_BUFFER_SIZE 1024
# define CGI_PATH "/www/html/cgi-bin"

# define MAX_CGI_TIME 1000000 

class CGI_Handler
{
	private:
		CGI_Handler();
		CGI_Handler(CGI_Handler &src);
		std::map<std::string, std::string> 	_env;

		std::string	_body;
		sConnection	&_conn;

	public:
		~CGI_Handler();
		CGI_Handler(sConnection & connection);

		CGI_Handler	operator=(const CGI_Handler &src);

		char *file_path(const char *file);
		std::string request(std::string file_path);

		void	env(std::string key, std::string value);
		void	init_env(sConnection & connection, sRequest & request);
		char	**generate_argv(void);
		void	put_query_variables_into_env(std::map<std::string, std::string> query);

		char	**get_env() const;
		void	fill_response(sConnection &, sResponse &, std::string);
};

#endif
