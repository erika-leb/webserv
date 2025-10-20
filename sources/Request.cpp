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
	
	http = _http.action + " " + _http.pathfile;
	return http;
}

static void remove_blank(std::string& str) {
	str.erase(std::remove_if(str.begin(), str.end(), isblank), str.end());
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
		_http.pathfile += "index.html";
	// Check pathfile

	std::getline(_rawHttp, _http.protocol);
	remove_blank(_http.protocol);
	if (!_http.protocol.empty()) {
		if (_http.protocol != "HTTP/1.1")
			_valid = false;
	}
	else {
		_http.protocol += "HTTP/1.1";
	}
}

void Request::parseSpec() {
	std::string key, value;
	while (std::getline(_rawSpec, key, ':') && _rawSpec >> value) {
		remove_blank(value);
		_spec[key] = value;
	}
}

void Request::makeResponse() {

}

std::ostream& operator<<( std::ostream& flux, const Request& r ) {
	std::map<std::string, std::string> tmp = r.getSpec();
	flux << r.getHttp() << std::endl;
	for (std::map<std::string, std::string>::iterator it = tmp.begin(); it != tmp.end(); it++) {
		flux << it->first << ": " << it->second << std::endl;
	}
	
	return flux;
}