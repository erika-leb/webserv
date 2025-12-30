#ifndef CGI_HPP
# define CGI_HPP

#include "all.hpp"
#include "Request.hpp"
#include "Client.hpp"
#include "Directive.hpp"

class Client;
class Request;
class Directive;

class Cgi
{
private:
	Client&		_cli;

	std::string	_path, _method;
	std::string _queryString;
	std::string _serverName;
	int			_port;
	std::string _cgiHandler;
	int 		_pipeDes[2];
	std::string _reqBody;
	bool		_isBody;

public:
	Cgi( Request&, Client& );
	Cgi( Cgi& cpy );
	Cgi& operator=( Cgi& other );
	~Cgi();

	int		getFd( int );

	void makeEnv( std::vector<std::string>, std::vector<char *> );

	void	handleCGI_fork( int );
	int		handleCGI_pipe( int );

	std::string parseHeader( std::string&, size_t );
	std::string parseCgiOutput( std::stringstream& ss );

	static void		sigchld_handler( int sig );
};

#endif