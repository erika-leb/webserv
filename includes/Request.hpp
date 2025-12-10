#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

class Client;
class ServerConfig;
class LocationConfig;

struct StatusInfo {
	std::string path;
	std::string message;
	std::string conn;
};

class Request
{
private:
	Client&		_cli;
	ServerConfig &_serv;

	std::map<std::string, std::string>	_reqParam, _params;
	std::stringstream	_rawHttp;
	std::map<int, StatusInfo> _errorPath;
	std::string	_action, _pathfile, _file, _fileType, _connection, _statusMess, _location;
	size_t		_sCode, _fileLength;
	int 			_locationIndex;
	std::vector<LocationConfig> _locs;
	std::stringstream _htmlList;

public:
	Request( Client& );
	Request( const Request& );
	Request& operator=( const Request& other );
	~Request();

	void getWriteLocation(std::string &pathfile);

	std::string							getPathFile() const;
	std::string							getAction() const;
	std::map<std::string, std::string>	getSpec() const;
	int 								getsCode() const;
	std::string 						getServIp() const;
	int									getServPort() const;

	bool is_cgi( std::string );

	void parseHttp();

	void generateHtml(std::string uri, std::string path);
	std::string getFile( std::string &pathfile, size_t* fileLength );
	void getPath(std::string &pathfile);
	void checkIndex();
	void checkPath( std::string pathfile, size_t& eCode );
	std::string ifError( std::string& path, std::string& con, int sCode );

	void setErrorPath();

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