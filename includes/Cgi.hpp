#ifndef CGI_HPP
# define CGI_HPP

#include "all.hpp"
#include "Request.hpp"

class Cgi
{
private:
	Client& _cli;

	std::string 		_path;
	int 				_pipeDes[2];
public:
	Cgi( std::string URI, Client& cli );
	Cgi( Cgi& cpy );
	Cgi& operator=( Cgi& other );
	~Cgi();

	void 		handleCGI_fork( int pollfd, struct epoll_event events[SOMAXCONN] );
	std::string handleCGI_pipe( int pipefd );

	static void		sigchld_handler( int sig );
};

#endif