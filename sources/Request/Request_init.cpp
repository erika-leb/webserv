#include "Request.hpp"
#include "all.hpp"

std::string Request::toLower(std::string &str)
{
	for (std::string::size_type i = 0; i < str.size(); i++)
		str[i] = std::tolower(str[i]);
	return (str);
}

Request::Request(Client &cli) : _cli(cli), _serv(_cli.getServ()), _chunked(0), _expect(0), _contentLength(0), _locationIndex(-1)
{
	std::stringstream ss(""), rawParam;
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
	trim_line(tmp);
	_rawHttp << tmp;
	while (std::getline(ss, tmp))
	{
		trim_line(tmp);
		rawParam << tmp;
	}
	while (std::getline(rawParam, key, ':') && rawParam >> value)
	{
		trim_line(key);
		trim_line(value);
		_reqParam[toLower(key)] = value;
	}
	getWriteLocation(_pathfile);
	setErrorPath();
}

Request::Request(const Request &cpy) : _cli(cpy._cli), _serv(cpy._serv),
	 _chunked(cpy._chunked), _expect(cpy._expect), _contentLength(cpy._contentLength), _locationIndex(cpy._locationIndex)
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
		_expect = other._expect;
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

std::string Request::getCgiHandler( std::string extension) {
	std::vector<Directive> dirs;

	if (_locationIndex != -1)
		dirs = _locs[_locationIndex].getDir();
	else
		dirs = _serv.getDir();

	for (std::vector<Directive>::size_type i = 0; i < dirs.size(); i++) {
		if (dirs[i].getName() == "cgi_handler") {
			if (dirs[i].getArg()[0] == extension)
				return dirs[i].getArg()[1];
		}
	}
	return "";
}

std::string Request::getBody() const {
	return _body.str();
}

size_t Request::getChunked() const
{
	return (_chunked);
}

size_t Request::getExpect() const
{
	return (_expect);
}


void Request::setCode(int n)
{
	_sCode = n;
}


void Request::setChunked(size_t n)
{
	_chunked = n;
}
