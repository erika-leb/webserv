#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <vector>

// faut ajouter un tableau ou je mets les fd aueje suis et la derniere connection pour tej ceux qui sont inactifs et les tej tous a la fin

class Server {

	public:
		Server();
		~Server();

		void launch();

	private:

		int _poll;
		int _fdListen;
		struct epoll_event _events[SOMAXCONN];
		std::vector< std::pair<int, int> > _listSock; //liste des sockets ouverts et leur derniere connection

		Server(const Server &src);
		Server &operator=(const Server &rhs);
} ;

void	*ft_memset(void *b, int c, size_t len);
size_t	ft_strlen(const char *str);
int		make_non_blocking(int sockfd);


int		is_fd_open(int fd); // a supprimer plus tard

#endif
