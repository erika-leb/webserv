#include "Client.hpp"

// Client::Client(int fd, ListenInfo info) : _fd(fd), _buff(""), _sendBuff(""), _lastConn(std::time(NULL)), _ip(info.ip), _port(info.port)
Client::Client(int fd, ServerConfig &conf) : _fd(fd), _buff(""), _sendBuff(""), _lastConn(std::time(NULL)), _conf(conf)
{
}

Client::Client(const Client &src): _fd(src._fd), _buff(src._buff), _sendBuff(src._sendBuff), _lastConn(src._lastConn), _conf(src._conf)
// Client::Client(const Client &src): _fd(src._fd), _buff(src._buff), _sendBuff(src._sendBuff), _lastConn(src._lastConn), _ip(src._ip), _port(src._port)
{
}

Client::~Client()
{
	close(_fd);
}

Client &Client::operator=(const Client &src)
{
	if (this != &src) {
		_fd = src._fd;
		_buff = src._buff;
		_sendBuff = src._sendBuff;
		_lastConn = src._lastConn;
		// _ip = src._ip;
		// _port = src._port;
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

void Client::setlastConn(time_t t)
{
	_lastConn = t;
}

time_t Client::getlastConn()
{
	return (_lastConn);
}

ServerConfig &Client::getServ()
{
	return (_conf);
}

// std::string &Client::getIp()
// {
// 	return (_ip);
// }

// int Client::getPort()
// {
// 	return (_port);
// }

