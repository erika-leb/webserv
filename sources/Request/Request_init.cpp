#include "Request.hpp"
#include "all.hpp"

Request::Request(Client &cli) : _cli(cli), _serv(_cli.getServ()),
	_locationIndex(-1)
{
	std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp;
	_locs = _serv.getLocation();
	_sCode = 200;
	_connection = "keep-alive";
	cli.setCon(true);
	std::getline(ss, tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp))
	{
		rawParam << tmp;
	}
	while (std::getline(rawParam, key, ':') && rawParam >> value)
	{
		remove_blank(value);
		_reqParam[key] = value;
	}
	setErrorPath();
}

Request::Request(const Request &cpy) : _cli(cpy._cli), _serv(cpy._serv),
	_locationIndex(cpy._locationIndex)
{
	_reqParam = cpy._reqParam;
	_errorPath = cpy._errorPath;
	_locs = cpy._locs;
}

Request &Request::operator=(const Request &other)
{
	if (this != &other)
	{
		_reqParam = other._reqParam;
		_errorPath = other._errorPath;
		_locationIndex = other._locationIndex;
		_locs = other._locs;
	}
	return (*this);
}

Request::~Request()
{
}

std::string Request::getPathFile(void) const
{
	return (_pathfile);
}

std::string Request::getAction(void) const
{
	return (_action);
}

std::map<std::string, std::string> Request::getSpec(void) const
{
	return (_reqParam);
}

int Request::getsCode(void) const
{
	return (_sCode);
}

std::string Request::getServIp( void ) const {
	return (_serv.getIp());
}

int	Request::getServPort( void ) const {
	return (_serv.getPort());
}

bool Request::is_cgi(std::string cgiFolder)
{
	if (_pathfile.find(cgiFolder) != std::string::npos)
		return (true);
	else
		return (false);
}
