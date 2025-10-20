#ifndef REQUEST_HPP
# define REQUEST_HPP

#include "all.hpp"
#include "Client.hpp"

class Request
{
private:
	std::map<std::string, std::string> _spec;
	std::string http;
public:
	Request( Client& cli );
	Request( const Request& cpy );
	Request& operator=( const Request& other );
	~Request();

	std::map<std::string, std::string> getContainer() const;
};

std::ostream& operator<<( std::ostream& flux, const Request& r );

#endif