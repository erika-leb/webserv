#include "all.hpp"
#include "Request.hpp"

/*
static bool startWith( std::string& str, std::string prefix ) {
	if (prefix.size() > str.size())
		return false;
	return str.compare(0, prefix.size(), prefix) == 0;
}
*/

static std::string getFile( std::string &pathfile, size_t* fileLength ) {
	std::fstream	fs;
	std::string		tmp, res;

	// pathfile modify according to root directory in configuration file
	if (pathfile.find(".html") != std::string::npos)
		pathfile.insert(1, "/html");
	fs.open(pathfile.c_str(), std::ios::in);
	if (fs.is_open()) {
		while (std::getline(fs, tmp)) {
			res += tmp;
			tmp.clear();
		}
		fs.close();
		*fileLength = res.size();
	}
	else {
		DEBUG_MSG("ERROR: Couldn't open file [" << pathfile << "]");
		res.clear();
	}
	
	return res;
}

/* This function can be reduce with a hash table */
static std::string ifError( std::string& path, std::string& con, int sCode ) {
	std::string str;

	// The path to error code must adapt to configuration file
	switch (sCode)
	{
	case 201:
		// path = newly created ressource
		str = " Created"; break;
	case 204:
		// no path
		str = " No content"; break;
	case 400:
		path = "./errors/400.html";
		str = " Bad request"; con = "close"; break;
	case 403:
		path = "./errors/403.html";
		str = " Forbidden"; break;
	case 404:
		path = "./errors/404.html";
		str = " Not found"; break;
	case 405:
		path = "./errors/405.html";
		str = " Method not allowed"; break; // Connection normally keep-alive
	case 500:
		path = "./errors/500.html";
		str = " Internal server error"; con = "close"; break;
	default:
		str = " Ok"; con = "keep-alive"; break;
	}

	return str;
}

static void checkPath( std::string pathfile, size_t& eCode ) {
	// Statement to adapt with configuration file
	if (pathfile.find(".html") != std::string::npos)
		pathfile.insert(0, "./html");
	else
		pathfile.insert(0, "."); // insert root directory cf. configuration file

	if (access(pathfile.c_str(), F_OK) < 0) {
		eCode = 404;
	}
	else if ((pathfile.find("/errors/")) != std::string::npos) {
		eCode = 403;
	}

	return ;
}

Request::Request( Client& cli ): _cli(cli) {
	std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp;

	_sCode = 200;
	_connection = "keep-alive";
	std::getline(ss, tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp)) {
		rawParam << tmp;
	}
	while (std::getline(rawParam, key, ':') && rawParam >> value) {
		remove_blank(value);
		_reqParam[key] = value;
	}
}

Request::Request( const Request& cpy ): _cli(cpy._cli) {
	_reqParam = cpy._reqParam;
}

Request& Request::operator=( const Request& other ) {
	if (this != &other)
		_reqParam = other._reqParam;
	return *this;
}

Request::~Request() {}

std::string Request::getPathFile( void ) const {
	return _pathfile;
}

std::string Request::getAction( void ) const {
	return _action;
}

std::map<std::string, std::string> Request::getSpec( void ) const {
	return _reqParam;
}

bool Request::is_cgi( std::string cgiFolder ) {

	if (_pathfile.find(cgiFolder) != std::string::npos)
		return true;
	else
		return false;
}

void Request::parseHttp( void ) {
	std::string	tmp;

	std::getline(_rawHttp, _action, ' ');
	remove_blank(_action);
	if (_action != "GET" &&
			_action != "POST" &&
			_action != "DELETE") {
		_sCode = 405;
	}
	
	std::getline(_rawHttp, _pathfile, ' ');
	remove_blank(_pathfile);
	DEBUG_MSG("file: " << _pathfile);
	if (_pathfile.empty())
		_sCode = 400;
	else {
		checkPath(_pathfile, _sCode);
	}
	_pathfile.insert(0, ".");

	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty()) {
		if (tmp != "HTTP/1.1") {
			_sCode = 400;
		}
	}
}

void Request::fGet( void ) {
	ifError(_pathfile, _connection, _sCode);
	_file = getFile(_pathfile, &_fileLength);
	if (_file.empty()) {
		_sCode = 403;
		ifError(_pathfile, _connection, _sCode);
		_file = getFile(_pathfile, &_fileLength);
	}
}

void Request::fPost( void ) {
	DEBUG_MSG("POST request");
	_sCode = 201;
	fGet();
}

void Request::fDelete( void ) {
	DEBUG_MSG("DELETE request");
	if (access(_pathfile.c_str(), W_OK) == 0) {
		std::remove(_pathfile.c_str());
		_sCode = 204;
		return;
	}
	_sCode = 403; 
	fGet();
}

void Request::handleAction( std::string action ) {
	std::string check[3] = {"GET", "POST", "DELETE"};
	void (Request::*f[3]) ( void ) = {&Request::fGet, &Request::fPost, &Request::fDelete};

	int i;
	if (_sCode == 200) {
		for (i = 0; i < 3; i++) {
			if (check[i] == action) {
				(this->*f[i])();
				break;
			}
		}
		// Next statement useless ?
		if (i == 3) {
			DEBUG_MSG("Not known method");
			fGet();
		}
	}
	else {
		DEBUG_MSG("Error code: " << _sCode);
		fGet();
	}
}

std::string Request::makeResponse( void ) {
	std::ostringstream mess;

	mess << "HTTP/1.1" << " " << _sCode << ifError(_pathfile, _connection, _sCode) << ENDLINE;
	mess << "Date: " << date(HTTP) << ENDLINE;
	mess << "Server: " << "localhost" << ENDLINE; // Modify according configuration file / fetch the host of the request
	mess << "Connection: " << _connection << ENDLINE; // Modify either the connection need to be maintained or not
	if (_sCode != 204) {
		mess << "Content-Type: " << "text/html" << ENDLINE; // Modify according to file
		mess << "Content-Length: " << _fileLength << ENDLINE;
		mess << ENDLINE;
		mess << _file;
	}
	else
		mess << ENDLINE;

	_cli.setSendBuff(mess.str());
	if (_connection == "keep-alive")
		_cli.setCon(true);
	else
		_cli.setCon(false);
	return _pathfile;
}
