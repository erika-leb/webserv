#ifndef SERVER_HPP
# define SERVER_HPP

#include "all.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include "GlobalConfig.hpp"
// faut ajouter un tableau ou je mets les fd aueje suis et la derniere connection pour tej ceux qui sont inactifs et les tej tous a la fin

// struct Client {
// 	int fd;
// 	std::string buff;
// 	bool request_complete;
// } ; //rajouter date de derniere connexion plus tard

class GlobalConfig;

class Server {

	public:
		Server(GlobalConfig *config);
		~Server();

		void launch();

		void initFdListen(int fd, int port);

		// void add_client(int fd, std::string str, bool d);
		static void handleSigint(int sig);
		void modifyEvent(int fd, uint32_t events);
		void deleteSocket(int client_fd);
		bool is_listen_fd(int fd);
		void NewIncomingConnection(int fd, struct sockaddr_in cli, struct epoll_event &event);

	private:

		GlobalConfig *config;

		static volatile sig_atomic_t flag;
		int _poll;
		std::vector<int> _fdListen;
		// int _fdListen;
		struct epoll_event _events[SOMAXCONN];
		// std::vector< Client *> _clients; //liste des sockets ouverts et leur derniere connection
		std::vector< Client *> _clients; //liste des sockets ouverts et leur derniere connection

		Server(const Server &src);
		Server &operator=(const Server &rhs);
} ;

#endif
