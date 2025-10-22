#include "Request.hpp"

Request::Request( Client& cli ): _cli(cli) {
	std::stringstream ss(cli.getBuff());
	std::string key, value, tmp;

	std::getline(ss, tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp)) {
		_rawSpec << tmp;
	}
}

Request::Request( const Request& cpy ): _cli(cpy._cli) {
	_spec = cpy._spec;
}

Request& Request::operator=( const Request& other ) {
	if (this != &other)
		_spec = other._spec;
	return *this;
}

Request::~Request() {}

std::map<std::string, std::string> Request::getSpec() const {
	return _spec;
}

std::string Request::getHttp() const {
	std::string http;
	
	http = _http.action + " " + _http.pathfile + " " + _http.protocol;
	return http;
}

static bool isBlank( char c ) {
	return std::isspace(static_cast<unsigned char>(c));
}

static void remove_blank(std::string& str) {
	str.erase(std::remove_if(str.begin(), str.end(), isBlank), str.end());
}

static std::string getFile(std::string pathfile, size_t* fileLength) {
	std::fstream	fs;
	std::string		tmp, res, finalPathfile;

	finalPathfile = "." + pathfile;
	fs.open(finalPathfile.c_str(), std::ios::in);
	if (fs.is_open()) {
		while (std::getline(fs, tmp)) {
			res += tmp;
			tmp.empty();
		}
		fs.close();
		*fileLength = res.size();
	}
	else
		std::cout << "file not found" << std::endl;
	
	return res;
}

void Request::parseHttp() {
	std::getline(_rawHttp, _http.action, ' ');
	remove_blank(_http.action);
	if (_http.action != "GET" ||
			_http.action != "POST" ||
			_http.action != "DELETE")
		_valid = false;
	
	std::getline(_rawHttp, _http.pathfile, ' ');
	remove_blank(_http.pathfile);
	if (!_http.pathfile.empty())
		_valid = false;
	/* Check pathfile */ 

	std::getline(_rawHttp, _http.protocol);
	remove_blank(_http.protocol);
	if (!_http.protocol.empty()) {
		if (_http.protocol != "HTTP/1.1")
			_valid = false;
	}
	else {
		_http.protocol += "HTTP/1.1 empty";
	}
}

void Request::parseSpec() {
	std::string key, value;
	while (std::getline(_rawSpec, key, ':') && _rawSpec >> value) {
		remove_blank(value);
		_spec[key] = value;
	}
}

static std::string date() {
	std::time_t date = std::time(0);
	std::tm* gmt = std::gmtime(&date);

	char buff[80];
	std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", gmt);

	return std::string(buff);
}

std::string Request::makeResponse() {
	_http.statusCode = 200;
	std::ostringstream mess(""), tmp;
	_file = getFile(_http.pathfile.c_str(), &_fileLength);

	tmp << _http.protocol << " " << _http.statusCode << " OK" << "\r\n";
	mess.clear();
	mess << tmp.str();
	mess << "Date: " << date() << "\r\n";
	mess << "Server: " << "localhost" << "\r\n";
	mess << "Connection: " << "keep-alive" << "\r\n";
	mess << "Content-Type: " << "type/html" << "\r\n";
	mess << "Content-Length: " << _fileLength << "\r\n";
	mess << "\r\n";
	mess << _file;

	std::cout << "[DEBUG] {\n";
	std::cout << mess.str();
	std::cout << "\n}\n";

	_cli.setSendBuff(mess.str());
	return mess.str();
}

std::ostream& operator<<( std::ostream& flux, const Request& r ) {
	std::map<std::string, std::string> tmp = r.getSpec();
	flux << r.getHttp() << std::endl;
	for (std::map<std::string, std::string>::iterator it = tmp.begin(); it != tmp.end(); it++) {
		flux << it->first << ": " << it->second << std::endl;
	}
	
	return flux;
}