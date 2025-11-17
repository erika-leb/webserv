#ifndef ALL_HPP
# define ALL_HPP

#ifdef DEBUG
	#define DEBUG_MSG(msg) do { \
		std::ostringstream _debug_oss; \
		_debug_oss << "[DEBUG] " << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "(): " << msg << std::endl; \
		std::cerr << _debug_oss.str(); \
	} while (0)
#else
	# define DEBUG_MSG(msg) do {} while (0)
#endif


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
#include <fstream>
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

// #include <errno

// #define MAXLINE 10
#define MAXLINE 4096
#define ENDLINE "\r\n"

enum dcode {
	HTTP,
	LOG
};


void		*ft_memset(void *b, int c, size_t len);
size_t		ft_strlen(const char *str);
int			make_non_blocking(int sockfd);
bool		isBlank( char c );
void		remove_blank( std::string& str );
std::string date(int format);
void 		trim_line(std::string &line);


int		is_fd_open(int fd); // a supprimer plus tard

#endif
