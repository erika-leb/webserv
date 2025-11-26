#include "Client.hpp"

Client::Client(int fd) : _fd(fd), _buff(""), _sendBuff(""), _cgi(NULL)
{
}

Client::Client(const Client &src): _fd(src._fd), _buff(src._buff), _sendBuff(src._sendBuff)
{
}

Client::~Client()
{
	close(_fd);
	if (_cgi)
		delete _cgi;
}

Client &Client::operator=(const Client &src)
{
	if (this != &src) {
		_fd = src._fd;
		_buff = src._buff;
		_sendBuff = src._sendBuff;
	}
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

void Client::addToSend()
{
	const char* resp = "HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Content-Length: 12\r\n"
	"\r\n"
	"Hello world!\n";

	_sendBuff.append(static_cast<std::string>(resp));
}

void Client::setCon( bool set ) {
	_con = set;
}

bool Client::isCon() {
	return _con;
}

void Client::setCgi( Cgi* newCgi ) {
	if (_cgi)
		delete _cgi;
	_cgi = newCgi;
}

Cgi* Client::getCgi() {
	return (_cgi);
}
