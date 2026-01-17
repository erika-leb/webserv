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
#include <string>
#include <stdexcept>
#include <limits>
#include <cctype>
#include <iterator>
#include <set>

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
#include <sys/stat.h>
#include <string>
#include <dirent.h>

#define MAXLINE 4096
#define ENDLINE "\r\n"
#define READ	0
#define WRITE	1
#define TIMEOUT_SECONDS 60
#define CGI_TIMEOUT_SECONDS 10

#define ERROR_400 "/errors/400.html"
#define ERROR_403 "/errors/403.html"
#define ERROR_404 "/errors/404.html"
#define ERROR_405 "/errors/405.html"
#define ERROR_411 "/errors/411.html"
#define ERROR_413 "/errors/413.html"
#define ERROR_500 "/errors/500.html"
#define ERROR_501 "/errors/501.html"

#define REDIR_301 "/redirections/301.html"
#define REDIR_302 "/redirections/302.html"
#define REDIR_307 "/redirections/307.html"
#define REDIR_308 "/redirections/308.html"

#define ROOT "html"

const std::string ROOT_STR = "default_documents";

struct docType {
	const char *ext;
	const char *type;
};

static docType gType[] = {
	{".html", "text/html"},
	{".css",  "text/css"},
	{".js",   "application/javascript"},
	{".png",  "image/png"},
	{".jpg",  "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".gif",  "image/gif"},
	{".ico",  "image/x-icon"},
	{".txt",  "text/plain"},
	{".json", "application/json"},
	{NULL, NULL}
};

enum dcode {
	HTTP,
	LOG
};

class Directive;

void		*ft_memset(void *b, int c, size_t len);
size_t		ft_strlen(const char *str);
int			make_non_blocking(int sockfd);
std::string date( int format );
void 		trim_line(std::string &line);

bool		isBlank( char c );
void		remove_blank( std::string& str );

bool		isDirectivePresent(std::string name, std::vector<Directive> dir);
Directive 	getDirective(std::string name, std::vector<Directive> dir);

int			is_fd_open(int fd);

#endif
