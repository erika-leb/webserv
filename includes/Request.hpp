#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

typedef struct s_http
{
	std::string	action;
	std::string	pathfile;
	std::string	protocol;
	size_t		statusCode;
} t_http;

class Request
{
private:
	std::map<std::string, std::string>	_spec;
	std::stringstream _rawSpec;
	std::stringstream _rawHttp;
	t_http		_http;
	Client&		_cli;
	bool		_valid;
	std::string	_file;
	size_t		_fileLength;

public:
	Request( Client& cli );
	Request( const Request& cpy );
	Request& operator=( const Request& other );
	~Request();

	std::map<std::string, std::string> getSpec() const;
	std::string getHttp() const;

	void parseHttp();
	void parseSpec();

	std::string makeResponse();
};

std::ostream& operator<<( std::ostream& flux, const Request& r );

#endif