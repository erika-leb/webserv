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
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
// #include <stdio.h>

// #include <errno

// #define MAXLINE 10
#define MAXLINE 4096
#define ENDLINE "\r\n"
#define READ	0
#define WRITE	1
#define TIMEOUT_SECONDS 60

#define ERROR_400 "/errors/400.html" //potentiel soucis si on est pas dans htlm ?? PROTECTION SI FICHIERS CHANGES DE PLACE ???
#define ERROR_403 "/errors/403.html"
#define ERROR_404 "/errors/404.html"
#define ERROR_405 "/errors/405.html"
#define ERROR_500 "/errors/500.html"

#define REDIR_301 "/redirections/301.html"
#define REDIR_302 "/redirections/302.html"
#define REDIR_307 "/redirections/307.html"
#define REDIR_308 "/redirections/308.html"

#define ROOT "html"

const std::string ROOT_STR = "html";

enum dcode {
	HTTP,
	LOG
};

// struct ListenInfo {
// 	// int fd;
// 	std::string ip;
// 	int port;
// };

class Directive;

void		*ft_memset(void *b, int c, size_t len);
size_t		ft_strlen(const char *str);
int			make_non_blocking(int sockfd);
std::string date( int format );
void 		trim_line(std::string &line);
bool		isDirectivePresent(std::string name, std::vector<Directive> dir);
Directive 	getDirective(std::string name, std::vector<Directive> dir);

bool		isBlank( char c );
void		remove_blank( std::string& str );

int			is_fd_open(int fd); // a supprimer plus tard

#endif
