#ifndef ALL_HPP
# define ALL_HPP

/* C++ headers */
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <csignal>
#include <sstream>
#include <fstream>
#include <cerrno>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cstdlib>

/* C headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>

# define CLOSED -2
#define MAXLINE 4096

void	*ft_memset(void *b, int c, size_t len);
size_t	ft_strlen(const char *str);
int		make_non_blocking(int sockfd);
bool	isBlank( char c );
void	remove_blank( std::string& str );
std::string date();


int		is_fd_open(int fd); // a supprimer plus tard

#endif
