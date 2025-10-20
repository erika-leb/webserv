#ifndef SERVER_HPP
# define SERVER_HPP

#include "all.hpp"
#include "Client.hpp"
#include "Request.hpp"
// faut ajouter un tableau ou je mets les fd aueje suis et la derniere connection pour tej ceux qui sont inactifs et les tej tous a la fin

// struct Client {
// 	int fd;
// 	std::string buff;
// 	bool request_complete;
// } ; //rajouter date de derniere connexion plus tard

class Server {

	public:
		Server();
		~Server();

		void launch();

		// void add_client(int fd, std::string str, bool d);
		static void handleSigint(int sig);
		void modifyEvent(int fd, uint32_t events);

	private:

		static volatile sig_atomic_t flag;
		// static volatile sig_atomic_t nbServ;
		int _poll;
		int _fdListen;
		struct epoll_event _events[SOMAXCONN];
		std::vector< Client *> _clients; //liste des sockets ouverts et leur derniere connection

		Server(const Server &src);
		Server &operator=(const Server &rhs);
} ;

void	*ft_memset(void *b, int c, size_t len);
size_t	ft_strlen(const char *str);
int		make_non_blocking(int sockfd);


int		is_fd_open(int fd); // a supprimer plus tard

#endif
