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
	// std::map<int, std::string>			_errorPath;
	std::map<int, StatusInfo> _errorPath;
	std::string							_action, _pathfile, _file, _fileType, _connection, _statusMess, _location;
	size_t								_sCode, _fileLength, _chunked; //3 derniers = 1 si chunked, uplaod renseigne et contentleingt active
	unsigned long long					_contentLength;
	int 								_locationIndex;
	std::vector<LocationConfig>			_locs;
	std::stringstream					_htmlList;
	std::stringstream					_body;

public:

	// init functions (Request_init.cpp)

	Request( Client& );
	Request( const Request& );
	Request& operator=( const Request& other );
	~Request();

	std::string toLower(std::string &str);

	std::string							getPathFile() const;
	std::string							getAction() const;
	std::map<std::string, std::string>	getSpec() const;
	int 								getsCode() const;
	std::string 						getServIp() const;
	int									getServPort() const;
	unsigned long long					getLenght() const;
	// unsigned long long	getBodyRead();

	bool is_cgi( std::string );

	// configuration implementation (Request_conf.cpp)

	void checkRedirAndMethod();
	void generateHtml(std::string uri, std::string path);
	void checkIndex();

	// function to get path (Request_path.cpp)

	void getWriteLocation(std::string &pathfile);
	void getPath(std::string &pathfile);
	std::string getFile( std::string &pathfile, size_t* fileLength );
	void checkPath( std::string pathfile, size_t& eCode );


	// error functions (Request_error.cpp)
	void setErrorPath();
	std::string ifError( std::string& path, std::string& con, int sCode );

	// parse functions (Request_parse.cpp)

	void parseParam(void);
	void parseHttp();
	void parseBody();


	// actions functions (request_action.cpp)

	int checkPostPath(Directive &directive);

	void fGet();
	void fPost();
	void fDelete();

	void handleAction( std::string );
	std::string makeResponse();
};

#endif
