#include "Request.hpp"

Request::Request( Client& cli ) {
	std::stringstream ss(cli.getBuff());
	std::string res;
}

Request::Request( const Request& cpy ) {
	_spec = cpy._spec;
}

Request& Request::operator=( const Request& other ) {
	if (this != &other)
		_spec = other._spec;
	return *this;
}

Request::~Request() {}

std::map<std::string, std::string> Request::getContainer() const {
	return _spec;
}

std::ostream& operator<<( std::ostream& flux, const Request& r ) {
	std::map<std::string, std::string>::iterator it = r.getContainer().begin();

	for (it; it != r.getContainer().end(); it++) {
		flux << it->first << ": " << it->second << std::endl;
	}
	
	return flux;
}