#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _buff("")
{
}

Client::Client()
{
}

Client::Client(const Client &src)
{
	(void) src;
}

Client::~Client()
{
	close(_fd);
}

Client &Client::operator=(const Client &src)
{
	(void)src;
	return (*this);
}

int Client::getFd() const
{
	return (_fd);
}

void Client::addBuff(char *str)
{
	_buff.append(str); //doute ici
}

std::string Client::getBuff(void) const
{
	return (_buff);
}
