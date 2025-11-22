#ifndef SERVER_HPP
# define SERVER_HPP

#include "all.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include "GlobalConfig.hpp"

class GlobalConfig;

class Server {

	public:
		Server(GlobalConfig *config);
		~Server();

		static void handleSigint(int sig);

		void launch();
		void initFdListen(int fd, int port, std::string &ip);
		void modifyEvent(int fd, uint32_t events);
		void deleteSocket(int client_fd);
		bool is_listen_fd(int fd);
		void NewIncomingConnection(int fd, struct sockaddr_in cli, struct epoll_event &event);
		int reveiveRequest(int i, std::string tmp);
		void prepareResponse(char buff[MAXLINE], std::string tmp, int client_fd, Client *cli);
		int sendRequest(int i, std::string tmp);
		unsigned long getIPAddr(std::string &ip, struct addrinfo **res);
		void cleanClose();

	private:

		GlobalConfig 			*config;

		static volatile sig_atomic_t flag;

		int 					_poll;
		std::vector<int> 		_fdListen;
		struct epoll_event 		_events[SOMAXCONN];
		std::vector< Client *>	_clients;

		Server(const Server &src);
		Server &operator=(const Server &rhs);
} ;

#endif
