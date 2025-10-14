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

// faut ajouter un tableau ou je mets les fd aueje suis et la derniere connection pour tej ceux qui sont inactifs et les tej tous a la fin

// class Server {

// 	public:
// 		Server();
// 		~Server();

// 	private:

// 		Server(const Server &src);
// 		Server &operator=(const Server &rhs);
// } ;

void	*ft_memset(void *b, int c, size_t len);
size_t	ft_strlen(const char *str);

#endif
