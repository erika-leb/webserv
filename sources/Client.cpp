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
	if (n <= static_cast<int>(_sendBuff.size()))
		_sendBuff.clear();
	else
		_sendBuff.erase(0, n);
}

size_t Client::setSendSize()
{
	return (_sendBuff.size());
}

void Client::setSendBuff( std::string str ) {
	_sendBuff = str;
}

std::string Client::getToSend()
{
	return (_sendBuff);
}

void Client::clearRequestBuff()
{
	size_t n;

	n = _buff.find("\r\n\r\n");
	_buff.erase(0, n + 4);
}
