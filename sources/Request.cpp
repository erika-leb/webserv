#include "Request.hpp"

Request::Request( const char *req ) {
	// parse req into map<key, value> of std::string
}

Request::Request( const Request& cpy ) {
	_spec = cpy._spec;
}

Request& Request::operator=( const Request& other ) {
	if (this != &other)
		_spec = other._spec;
	return *this;
}

std::ostream& operator<<( std::ostream& flux, const Request& r ) {
	std::map<std::string, std::string>::const_iterator it;
}