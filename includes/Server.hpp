#ifndef SERVER_HPP
# define SERVER_HPP

#include "all.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include "GlobalConfig.hpp"
#include "Cgi.hpp"
#include "ServerConfig.hpp"

class GlobalConfig;

class ServerConfig;

class Server {

	public:

		//init functions (server_init.cpp)

		Server(GlobalConfig *config);
		~Server();

		void initFdListen(int fd, int port, std::string &ip);

		// main functions (server.cpp)

		void NewIncomingConnection(int fd, struct sockaddr_in cli, struct epoll_event &event);
		void prepareResponse(char buff[MAXLINE], std::string& tmp, int client_fd, Client *cli, int n);
		int receiveRequest(int i, std::string& tmp);
		int sendRequest(int i, std::string tmp);

		void launch();

		// utils functions (server_utils.cpp)

		void cleanClose();

		unsigned long getIPAddr(std::string &ip, struct addrinfo **res);
		void modifyEvent(int fd, uint32_t events);
		void deleteSocket(int client_fd);
		bool is_listen_fd(int fd);
		bool is_body_complete( Client *cli );


		void checkTimeOut();
		int timeOut();

		void clearRequest(Client *cli, Request *req);

		// CGI functions
		bool is_pipe_fd( int );
		void receiveCgi( int , std::string );

		static void handleSigint(int sig);

	private:

		GlobalConfig 			*config;

		static volatile sig_atomic_t flag;

		int 						_poll;
		std::map<int, ServerConfig>	_fdListen;
		struct epoll_event			_events[SOMAXCONN];
		std::vector< Client *>		_clients;

		Server(const Server &src);
		Server &operator=(const Server &rhs);
} ;

#endif
