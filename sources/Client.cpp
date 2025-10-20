#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _buff(""), _sendBuff("")
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
	_buff.append(str);
}

std::string Client::getBuff(void) const
{
	return (_buff);
}

const char *Client::getSendBuffer()
{
	return (_sendBuff.c_str());
}

void Client::sendBuffErase(int n)
{
	if (n <= _sendBuff.size())
		_sendBuff.clear();
	else
		_sendBuff.erase(0, n);
}

size_t Client::setSendSize()
{
	return (_sendBuff.size());
}

std::string Client::getToSend()
{
	return (_sendBuff);
}
