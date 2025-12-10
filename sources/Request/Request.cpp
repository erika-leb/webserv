#include "Request.hpp"
#include "all.hpp"

// NB = pas trop de protection sur les getline

void Request::parseParam(void) //voir avec thibualt si besoin de faire le cas content-encoding
{
	std::string value;

	if (_reqParam.find("transfer-encoding") != _reqParam.end())
	{
		value = toLower(_reqParam["transfer-encoding"]);
		if (value ==  "chunked")
			_chunked = 1;
		else
		{
			_sCode = 501;
			return;
		}
	}

	if (_reqParam.find("content-length") != _reqParam.end())
	{
		value = _reqParam["content-length"];
		for (std::string::size_type i = 0; i < value.size(); i++)
		{
			if (value[i] < '0' || value[i] > '9')
			{
				_sCode = 400;
				return ;
			}
		}
	}
	else
	{
		if (_action == "{POST")
			_sCode = 411;
	}

	if (_reqParam.find("expect") != _reqParam.end())
	{
		value = toLower(_reqParam["expect"]);
		if (value == "100-continue")
			_sCode = 501;
		else
			_sCode = 400;
	}
}

void Request::parseHttp(void)
{
	std::string tmp;
	std::getline(_rawHttp, _action, ' ');
	remove_blank(_action);
	if (_action != "GET" && _action != "POST" && _action != "DELETE")
	{
		_sCode = 405;
	}
	std::getline(_rawHttp, _pathfile, ' ');
	DEBUG_MSG("pathfile brut = " << _pathfile);
	remove_blank(_pathfile);

		// Only for test purpose
	std::string pathWithoutQuery(_pathfile);
	size_t end;
	if ( (end = _pathfile.find('?')) != std::string::npos )
		pathWithoutQuery = _pathfile.substr(0, end);

	if (_pathfile.empty())
		_sCode = 400;
	else
	{
		checkRedirAndMethod();
		checkPath(_pathfile, _sCode);
	}
	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty())
	{
		if (tmp != "HTTP/1.1")
		{
			_sCode = 400;
		}
	}
	else
		_sCode = 400;
	DEBUG_MSG("path at end of parse = " + _pathfile);
	// std::cout << "ode =" << _sCode << std::endl;
	if (_sCode == 200)
		parseParam();
}

void Request::parseBody()
{
	std::string::size_type pos;
	Client& cli = _cli;

	pos = cli.getBuff().find("\r\n\r\n");
	if (pos + 4 < cli.getBuff().size())
		_body << cli.getBuff().substr(pos + 4);
	else
		_body << "";  // body vide
	// _body << cli.getBuff().substr(pos + 4);
}

void Request::fGet(void)
{
	DEBUG_MSG("GET request");
	ifError(_pathfile, _connection, _sCode);
	// std::cerr << "code = " << _sCode << ";, path = " << _pathfile << std::endl;
	_file = getFile(_pathfile, &_fileLength);
	if (_file.empty())
	{
		_sCode = 403;
		ifError(_pathfile, _connection, _sCode);
		_file = getFile(_pathfile, &_fileLength);
	}
	// std::cerr << "apres = code = " << _sCode << ";,path = " << _pathfile << std::endl;
}





void Request::fPost(void)
{
	DEBUG_MSG("POST request");
	struct stat st;
	std::fstream upload;
	// si il y a une erreur quelque part, changer le sCode et faire fGet
	//ici ou avant on parse le body (specificite content lenght et chunked)
	// processer l'info = cgi, uplaod si autoriser par la locaion

	if (stat(_pathfile.c_str(), &st) == -1)
	{
		if (errno != ENOENT)
		{
			_sCode = 403;
			fGet();
			return ;
		}
		std::string::size_type pos = _pathfile.find_last_of('/');
		if (pos == std::string::npos)
		{
			_sCode = 403;
			return;
		}

		std::string parent = _pathfile.substr(0, pos);

		if (access(parent.c_str(), W_OK) != 0)
		{
			_sCode = 403;
			return;
		}
	}
	else
	{
		if (access(_pathfile.c_str(), W_OK) != 0)
		{
			_sCode = 403;
			fGet();
			return ;
		}
	}
	upload.open(_pathfile.c_str(), std::ios::out | std::ios::trunc);
	if (!upload.is_open())
	{
		_sCode = 500;
		fGet();
		return;
	}
	upload << _body.str();
	_sCode = 201;
	// fGet(); // pourquoi il y a un fGet ? il faut l'enlever je crois
}

void Request::fDelete(void)
{
	DEBUG_MSG("DELETE request");
	getPath(_pathfile);
	std::cout << "delete pathfile : " << _pathfile << std::endl;
	if (access(_pathfile.c_str(), W_OK) == 0)
	{
		std::remove(_pathfile.c_str());
		_sCode = 204;
		return ;
	}
	_sCode = 403;
	fGet();
}

void Request::handleAction(std::string action)
{
	int	i;

	std::string check[3] = {"GET", "POST", "DELETE"};
	void (Request::*f[3])(void) = {&Request::fGet, &Request::fPost,
		&Request::fDelete};
	// DEBUG_MSG("CHOOSE Method " << _action);
	// DEBUG_MSG("path = " + _pathfile);
	std::cout << "codi = " << _sCode << std::endl;
	if (_sCode == 200)
	{
		for (i = 0; i < 3; i++)
		{
			if (check[i] == action)
			{
				(this->*f[i])();
				break ;
			}
		}
		// Next statement useless ?
		if (i == 3)
		{
			DEBUG_MSG("Not known method");
			fGet();
		}
	}
	else
	{
		DEBUG_MSG("Error code: " << _sCode);
		fGet();
	}
}

std::string Request::makeResponse(void)
{
	std::ostringstream mess;
	// ServerConfig conf = _cli.getServ();
	mess << "HTTP/1.1"
			<< " " << _sCode << ifError(_pathfile, _connection,
				_sCode) << ENDLINE;
	mess << "Date: " << date(HTTP) << ENDLINE;
	mess << "Server: " << _serv.getIp() << ":" << _serv.getPort() << ENDLINE;
		// Modify according configuration file / fetch the host of the request
	// mess << "Server: " << "localhost" << ENDLINE;
		// Modify according configuration file / fetch the host of the request
	mess << "Connection: " << _connection << ENDLINE;
		// Modify either the connection need to be maintained or not
	if (_sCode > 300 && _sCode < 400)
		mess << "Location: " << _location << ENDLINE;
	if (_sCode != 204)
	{
		mess << "Content-Type: "
				<< "text/html" << ENDLINE; // Modify according to file
		mess << "Content-Length: " << _fileLength << ENDLINE;
		mess << ENDLINE;
		mess << _file;
	}
	else
		mess << ENDLINE;
	if (_htmlList.str() == "")
	{
		std::cerr << "message envopye = " << mess.str() << std::endl;
		_cli.setSendBuff(mess.str());
	}
	else
	{
		std::cerr << "message envopye = " << _htmlList.str() << std::endl;
		_cli.setSendBuff(_htmlList.str());
	}
	if (_connection == "keep-alive")
		_cli.setCon(true);
	else
		_cli.setCon(false);
	return (_pathfile);
}
