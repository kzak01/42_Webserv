#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "../libraries/types.hpp"
#include "../libraries/includes.hpp"
#include "../libraries/classes.hpp"

class Client
{
	private:
        sSocket     _server;
        int         _create_socket();

	public:
		Client();
		~Client();

		int		start();

};

#endif
