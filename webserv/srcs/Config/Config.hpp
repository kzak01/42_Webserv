#ifndef CONFIG_HPP
# define CONFIG_HPP
# include "../../libraries/types.hpp"
# include "../../libraries/structures.hpp"
# include "../utils.hpp"
# include "../../libraries/includes.hpp"


struct sServ;
struct sLocation;
struct sMethods;

class Config {

	private:
		//FUNCTIONS THAT READ AND PARSE CONFIG FILE

		void	_read_config_file(std::string path);
		void    _file_check();

		void	_read_http_block();
		int		_parse_http_block();
		void	_read_server_block();
		int		_parse_server_block(sServ & serv_struct);
		void	_read_location_block(sServ & serv_struct, std::string & location_path);
		int		_parse_location_block(sServ & serv_struct, sLocation & location_struct, std::string & location_path);

		void	_read_server_block_directive(sServ & serv_struct, std::string directive);
		void	_read_location_block_directive(sLocation & location_struct, std::string directive);

		void	_split_directive_values(std::size_t values_end, std::vector<std::string> & vec);

		bool	_is_location_path_ok(std::string path);

		//TEMPLATE FUNCTIONS THAT STORE DIRECTIVE VALUES

		template<typename T>
		void	_store_simple_directive_value(T & structure, std::size_t value_end, std::string directive);
		template<typename T>
		void	_store_error_page_values(T & structure, std::size_t values_end);
		template<typename T>
		void	_store_client_max_body_size_value(T & structure, std::size_t values_end);
		template<typename T>
		void	_store_autoindex_value(T & structure, std::size_t value_end);
		template<typename T>
		void	_store_return_values(T & structure, std::size_t values_end);

		//FUNCTIONS THAT STORE DIRECTIVE VALUES

		void	_inherit_directives(sServ & serv_struct, sLocation & location_struct);

		void	_store_listen_values(sServ & serv_struct, std::size_t values_end);
		void	_store_server_name_values(sServ & serv_struct, std::size_t values_end);
		void	_store_cgi_enable_values(sServ & serv_struct, std::size_t values_end);

		
		void	_store_methods_values(sLocation & location_struct, std::size_t values_end);
		void	_store_alias_value(sLocation & location_struct, std::size_t value_end);

		//TOOL FUNCTIONS

		void	_cleanTrim(std::string & str);
		bool	_is_keywords(std::string word, std::vector<std::string> keywords) const;
		bool	_is_anything_inside_line();
		bool	_is_that_word_now(std::string word) const;
		sServ * _find_parent_server(sServ & serv_struct);
		bool	_are_all_servernames_different(std::vector<std::string> domain_names_1, std::vector<std::string> domain_names_2);
		
		//ENDING FUNCTION

		void	_error_close_ifstream();

		//EXCEPTIONS

		//exception from which all the other exceptions derive
		class ConfigParentException : public std::exception {
			public:
				ConfigParentException();
				virtual ~ConfigParentException() throw();
            	virtual std::string const configWhat(Config & config) const throw();
			protected:
				std::string _message;
				
		};

		class WrongConfFileException : public std::exception {
			public:
            	virtual const char *what() const throw();
		};

		// class NotEnoughInfoInConfFile : public std::exception {
		// 	public:
        //     	virtual const char *what() const throw();
		// };

		class WrongDirectiveException : public ConfigParentException {
			public:
				WrongDirectiveException();
		};

		class WrongValuesException : public ConfigParentException {
			public:
				WrongValuesException();
		};

		class MissingBracesException : public ConfigParentException {
			public:
				MissingBracesException();
		};

		class NoServerException : public std::exception {
			public:
            	virtual const char *what() const throw();
		};

		class MissingSemicolonException : public ConfigParentException {
			public:
				MissingSemicolonException();
		};

		class CharAfterSemicolonException : public ConfigParentException {
			public:
				CharAfterSemicolonException();
		};

		class WrongLocationPathException : public ConfigParentException {
			public:
				WrongLocationPathException();
		};

		class ServersConflictException : public std::exception {
			public:
            	virtual const char *what() const throw();
		};

		//ATTRIBUTES

		std::vector<sServ> &		_servers;

		std::vector<std::string>	_server_keywords;
		std::vector<std::string>	_location_keywords;		

		std::ifstream		_in_stream;
		std::string			_line;

		std::size_t			_lines_count;
		std::size_t			_position;
		std::size_t			_servers_count;
		
		bool				_server_block;
		bool				_server_brace;
		bool				_location_block;
		bool				_location_brace;

	public:
		Config(std::string const & path, std::vector<sServ> & servers);
		~Config();

		bool	conf_file_ok;// attribute that says if the parsing of the config file has success

};

# include "Config_templates.tpp"

#endif