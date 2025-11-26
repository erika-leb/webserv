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
	std::string	_action, _pathfile, _file, _fileType, _connection, _statusMess;
	size_t		_sCode, _fileLength;

public:
	Request( Client& );
	Request( const Request& );
	Request& operator=( const Request& other );
	~Request();

	std::string getPathFile() const;
	std::string getAction() const;
	std::map<std::string, std::string> getSpec() const;
	int getsCode() const;

	bool is_cgi( std::string );

	void parseHttp();

	void fGet();
	void fPost();
	void fDelete();

	void handleAction( std::string );
	std::string makeResponse();
};

#endif