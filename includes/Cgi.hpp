#ifndef CGI_HPP
# define CGI_HPP

#include "all.hpp"
#include "Request.hpp"
#include "Client.hpp"

class Client;

class Cgi
{
private:
	Client&		_cli;

	std::string	_path, _method;
	int 		_pipeDes[2];
	std::string _queryString;
public:
	Cgi( std::string URI, std::string method, Client& cli );
	Cgi( Cgi& cpy );
	Cgi& operator=( Cgi& other );
	~Cgi();

	int		getFd( int );

	std::vector<char *> makeEnv( void );

	void	handleCGI_fork( int );
	int		handleCGI_pipe( int );

	static void		sigchld_handler( int sig );
};

#endif