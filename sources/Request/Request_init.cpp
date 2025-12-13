#include "Request.hpp"
#include "all.hpp"

std::string Request::toLower(std::string &str)
{
	for (std::string::size_type i = 0; i < str.size(); i++)
		str[i] = std::tolower(str[i]);
	return (str);
}

Request::Request(Client &cli) : _cli(cli), _serv(_cli.getServ()), _chunked(0), _contentLength(0), _locationIndex(-1)
{
	std::stringstream ss(""), rawParam;
	// std::stringstream ss(cli.getBuff()), rawParam;
	std::string key, value, tmp, rawHeader;
	std::string::size_type pos;

	_locs = _serv.getLocation();
	_sCode = 200;
	_connection = "keep-alive";
	cli.setCon(true);
	pos = cli.getBuff().find("\r\n\r\n");
	rawHeader = cli.getBuff().substr(0, pos);
	ss << rawHeader;
	std::getline(ss, tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp))
	{
		rawParam << tmp;
	}
	while (std::getline(rawParam, key, ':') && rawParam >> value)
	{
		trim_line(key);
		trim_line(value);
		// remove_blank(value);
		// _reqParam[key] = value;
		_reqParam[toLower(key)] = value;
	}
	// for (std::map<std::string, std::string>::iterator it = _reqParam.begin(); it != _reqParam.end(); it++)
	// {
	// 	std::cout << "cle = " << it->first << " | valeur = " << it->second << std::endl;
	// 	// DEBUG_MSG("cle = " << it->first << " | valeur = " << it->second);
	// }
	// if (pos + 4 < cli.getBuff().size())
	// 	_body << cli.getBuff().substr(pos + 4);
	// else
	// 	_body << "";  // body vide
	// _body << cli.getBuff().substr(pos + 4);
	setErrorPath();
	DEBUG_MSG("	code fin init = " << _sCode);
}

Request::Request(const Request &cpy) : _cli(cpy._cli), _serv(cpy._serv),
	 _chunked(cpy._chunked), _contentLength(cpy._contentLength), _locationIndex(cpy._locationIndex)
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
		_chunked = other._chunked;
		_contentLength = other._contentLength;
		// _bodyRead = other._bodyRead;
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

unsigned long long	Request::getLenght() const
{
	return _contentLength;
}


bool Request::is_cgi(std::string cgiFolder)
{
	if (_pathfile.find(cgiFolder) != std::string::npos)
		return (true);
	else
		return (false);
}
