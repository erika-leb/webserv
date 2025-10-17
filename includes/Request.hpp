#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"

class Request
{
private:
	std::map<std::string, std::string> _spec;
public:
	Request( const char *req );
	Request( const Request& cpy );
	Request& operator=( const Request& other );
	~Request();
};

std::ostream& operator<<( std::ostream& flux, const Request& r );

#endif