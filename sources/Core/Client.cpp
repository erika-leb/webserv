#include "Client.hpp"

Client::Client(int fd, ServerConfig &conf) : _cgi(NULL), _req(NULL), _fd(fd), _buff(""), _sendBuff(""), _lastConn(std::time(NULL)), _conf(conf), _bodyRead(0)
{
}

Client::Client(const Client &src): _cgi(src._cgi), _req(src._req), _fd(src._fd), _buff(src._buff), _sendBuff(src._sendBuff), _lastConn(src._lastConn), _conf(src._conf), _bodyRead(src._bodyRead)
{
}

Client::~Client()
{
	close(_fd);
	if (_cgi)
		delete _cgi;
	if (_req)
		delete _req;
}

Client &Client::operator=(const Client &src)
{
	if (this != &src) {
		_cgi = src._cgi;
		_fd = src._fd;
		_buff = src._buff;
		_sendBuff = src._sendBuff;
		_lastConn = src._lastConn;
		_req = src._req;
		_bodyRead = src._bodyRead;
	}
	return (*this);
}


int Client::getFd() const
{
	return (_fd);
}


void Client::addBuff(char *str, size_t len)
{
	_buff.append(str, len);
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

void Client::setSendBuff( std::string str ) {
	_sendBuff = str;
}

void Client::clearRequestBuff(int f, unsigned long long nb)
{
	size_t n;

	if (f == 0)
	{
		n = _buff.find("\r\n\r\n");
		_buff.erase(0, n + 4);
	}
	else
		_buff.erase(0, nb);
}


size_t Client::setSendSize()
{
	return (_sendBuff.size());
}


std::string Client::getToSend()
{
	return (_sendBuff);
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

void Client::setCgiPid( pid_t pid ) {
	_cgiPid = pid;
}

pid_t Client::getCgiPid()
{
	return (_cgiPid);
}

void Client::deleteCgi() {
	if (_cgi) {
		delete _cgi;
		_cgi = NULL;
	}
}

Cgi* Client::getCgi() {
	return (_cgi);
}


Request* Client::getRequest() const
{
	return _req;
}

void Client::setRequest(Request *req)
{
	_req = req;
}
void Client::deleteRequest()
{
	delete _req;
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


unsigned long long	Client::getBodyRead()
{
	return _bodyRead;
}


void Client::setBodyRead(unsigned long long nb)
{
	_bodyRead = nb;
}
