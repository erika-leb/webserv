#include "all.hpp"
#include "Request.hpp"

static bool startWith( std::string& str, std::string prefix ) {
	if (prefix.size() > str.size())
		return false;
	return str.compare(0, prefix.size(), prefix) == 0;
}

static std::string getFile( std::string pathfile, size_t* fileLength ) {
	std::fstream	fs;
	std::string		tmp, res, finalPathFile;

	if (!startWith(pathfile, "./html"))
		finalPathFile += "./html";
	finalPathFile += pathfile;
	fs.open(finalPathFile.c_str(), std::ios::in);
	if (fs.is_open()) {
		while (std::getline(fs, tmp)) {
			res += tmp;
			tmp.clear();
		}
		fs.close();
		*fileLength = res.size();
	}
	else {
		std::cout << date(LOG) << ": ERROR: Couldn't open file [" << finalPathFile << "]" << std::endl;
		res.clear();
	}
	
	return res;
}

static std::string ifError( std::string& path, std::string& con, int sCode ) {
	std::string str;

	switch (sCode)
	{
	case 201:
		// path = newly created ressource
		str = " Created"; break;
	case 204:
		// no path
		str = " No content"; break;
	case 400:
		path = "/errors/400.html";
		str = " Bad request"; con = "close"; break;
	case 403:
		path = "/errors/403.html";
		str = " Forbidden"; break;
	case 404:
		path = "/errors/404.html";
		str = " Not found"; break;
	case 405:
		path = "/errors/405.html";
		str = " Method not allowed"; break; // Connection normally keep-alive
	case 500:
		path = "/errors/500.html";
		str = " Internal server error"; con = "close"; break;
	default:
		str = " Ok"; con = "keep-alive"; break;
	}

	return str;
}

Request::Request( Client& cli ): _cli(cli) {
	std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp;

	_valid = true;
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

std::map<std::string, std::string> Request::getSpec( void ) const {
	return _reqParam;
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
	if (_pathfile.empty())
		_valid = false;
	else {
		_pathfile.insert(0, "./html");
		if (access(_pathfile.c_str(), F_OK) < 0) {
			_sCode = 404;
		}
		else if ((_pathfile.find("/errors/")) != std::string::npos) {
			// std::cout << "[DEBUG] `/errors/' found in url :" << _pathfile.find("/errors/") << std::endl;
			_sCode = 403;
		}
		_pathfile.erase(0, 6);
	}

	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty()) {
		if (tmp != "HTTP/1.1") {
			_valid = false;
		}
	}
}

void Request::fGet( void ) {
	ifError(_pathfile, _connection, _sCode);
	_file = getFile(_pathfile.c_str(), &_fileLength);
	if (_file.empty()) {
		_sCode = 403;
		ifError(_pathfile, _connection, _sCode);
		_file = getFile(_pathfile.c_str(), &_fileLength);
	}
}

void Request::fPost( void ) {
	DEBUG_MSG("POST request");
	_sCode = 201;
	fGet();
}

void Request::fDelete( void ) {
	DEBUG_MSG("DELETE request");
	_sCode = 204;
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

	if (_valid == false) {
		_sCode = 400;
	}

	handleAction(_action);

	mess << "HTTP/1.1" << " " << _sCode << ifError(_pathfile, _connection, _sCode) << ENDLINE;
	mess << "Date: " << date(HTTP) << ENDLINE;
	mess << "Server: " << "localhost" << ENDLINE; // Modify according configuration file
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
