#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

class Client;
class ServerConfig;

class Request
{
private:
	Client&		_cli;
	ServerConfig &_serv;

	std::map<std::string, std::string>	_reqParam, _params;
	std::stringstream	_rawHttp;
	std::map<int, std::string> _errorPath;
	std::string	_action, _pathfile, _file, _fileType, _connection, _statusMess, _location;
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

	std::string getFile( std::string &pathfile, size_t* fileLength );
	void getPath(std::string &pathfile);
	void checkPath( std::string pathfile, size_t& eCode );
	std::string ifError( std::string& path, std::string& con, int sCode );

	void setErrorPath(int i);

	// bool IsMethodAllowed();
	// int IsRedir();
	void checkRedirAndMethod();
	// void setLocErrorPath(int i);

	void fGet();
	void fPost();
	void fDelete();

	void handleAction( std::string );
	std::string makeResponse();
};

#endif
