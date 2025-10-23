#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

class Request
{
private:
	Client&		_cli;

	std::map<std::string, std::string>	_reqParam, _params;
	std::stringstream	_rawHttp;
	std::string	_action, _pathfile, _file, _fileType;
	size_t		_sCode, _fileLength;
	bool		_valid;

public:
	Request( Client& cli );
	Request( const Request& cpy );
	Request& operator=( const Request& other );
	~Request();

	std::map<std::string, std::string> getSpec() const;
	std::string getHttp() const;


	void parseHttp();
	std::string makeResponse();
};

#endif