#include "all.hpp"
#include "Request.hpp"

static std::string getFile(std::string pathfile, size_t* fileLength) {
	std::fstream	fs;
	std::string		tmp, res, finalPathFile;

	// std::cout << "[DEBUG] *pathfile.begin():" << *pathfile.begin() << std::endl;
	if (*pathfile.begin() != '.')
		finalPathFile += '.';
	finalPathFile += pathfile;
	// std::cout << "[DEBUG] finalPathFile:" << finalPathFile << std::endl;
	fs.open(finalPathFile.c_str(), std::ios::in);
	if (fs.is_open()) {
		while (std::getline(fs, tmp)) {
			res += tmp;
			tmp.empty();
		}
		fs.close();
		*fileLength = res.size();
	}
	else {
		std::cout << "ERROR: Couldn't open file" << std::endl;
		res.empty();
	}
	
	return res;
}

static std::string ifError(std::string& path, int sCode ) {
	std::string str;

	switch (sCode)
	{
	case 400:
		path = "/errors/400.html";
		str = " Bad request"; break;
	case 403:
		path = "/errors/403.html";
		str = " Forbidden"; break;
	case 404:
		path = "/errors/404.html";
		str = " Not found"; break;
	case 405:
		path = "/errors/405.html"; break;
		str = " Method not allowed";
	default:
		str = " Ok"; break;
	}

	return str;
}

Request::Request( Client& cli ): _cli(cli) {
	std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp;

	_valid = true;
	_sCode = 200;
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

std::map<std::string, std::string> Request::getSpec() const {
	return _reqParam;
}

void Request::parseHttp() {
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
		_pathfile.insert(0, ".");
		if (access(_pathfile.c_str(), F_OK) < 0) {
			_sCode = 404;
		}
		else if ((_pathfile.find("/errors/")) != std::string::npos) {
			// std::cout << "[DEBUG] `/errors/' found in url :" << _pathfile.find("/errors/") << std::endl;
			_sCode = 403;
		}
	}

	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty()) {
		if (tmp != "HTTP/1.1") {
			_valid = false;
		}
	}
}

std::string Request::makeResponse() {
	std::ostringstream mess;

	if (_valid == false) {
		_sCode = 400;
	}

	std::string statusMess(ifError(_pathfile, _sCode));
	_file = getFile(_pathfile.c_str(), &_fileLength);

	mess << "HTTP/1.1" << " " << _sCode << statusMess << "\r\n";
	mess << "Date: " << date(HTTP) << "\r\n";
	mess << "Server: " << "localhost" << "\r\n"; // Modify according configuration file
	mess << "Connection: " << "keep-alive" << "\r\n"; // Modify either the connection need to be maintained or not
	// mess << "Connection: " << "close" << "\r\n"; // Modify either the connection need to be maintained or not
	mess << "Content-Type: " << "text/html" << "\r\n"; // Modify according to file
	mess << "Content-Length: " << _fileLength << "\r\n";
	mess << "\r\n";
	mess << _file;

	_cli.setSendBuff(mess.str());
	return _pathfile;
}
