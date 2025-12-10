#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

class Client;
class ServerConfig;
class LocationConfig;

class Request
{
private:
	Client&								_cli;
	ServerConfig &						_serv;

	std::map<std::string, std::string>	_reqParam, _params;
	std::stringstream					_rawHttp;
	std::map<int, std::string>			_errorPath;
	std::string							_action, _pathfile, _file, _fileType, _connection, _statusMess, _location;
	size_t								_sCode, _fileLength, _chunked; //3 derniers = 1 si chunked, uplaod renseigne et contentleingt active
	unsigned long long					_contentLength;
	int 								_locationIndex;
	std::vector<LocationConfig>			_locs;
	std::stringstream					_htmlList;
	std::stringstream					_body;

public:
	Request( Client& );
	Request( const Request& );
	Request& operator=( const Request& other );
	~Request();

	std::string toLower(std::string &str);

	std::string getPathFile() const;
	std::string getAction() const;
	std::map<std::string, std::string> getSpec() const;
	int getsCode() const;
	unsigned long long	getLenght();
	unsigned long long	getBodyRead();

	bool is_cgi( std::string );

	void checkRedirAndMethod();
	void generateHtlm(std::string uri, std::string path);
	void checkIndex();

	void getWriteLocation(std::string &pathfile);
	std::string getFile( std::string &pathfile, size_t* fileLength );
	void getPath(std::string &pathfile);
	void checkPath( std::string pathfile, size_t& eCode );

	void setErrorPath();
	std::string ifError( std::string& path, std::string& con, int sCode );

	void parseParam(void);
	void parseHttp();

	void fGet();
	void fPost();
	void fDelete();

	void parseBody();

	void handleAction( std::string );
	std::string makeResponse();
};

#endif
