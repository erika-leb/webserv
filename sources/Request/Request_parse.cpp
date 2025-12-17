#include "Request.hpp"
#include "all.hpp"

void Request::parseParam(void) //voir avec thibualt si besoin de faire le cas content-encoding
{
	std::string value;
	// perror("KATY");
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
		_contentLength = std::atoi(value.c_str());
	}
	else
	{
		if (_action == "POST")
			_sCode = 411;
	}

	perror("ici");
	if (_reqParam.find("expect") != _reqParam.end())
	{
		perror("la");
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
		DEBUG_MSG("action not allowed = " << _action);
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
		checkPath(pathWithoutQuery, _sCode);
		// checkPath(_pathfile, _sCode);
			getPath(_pathfile);
		DEBUG_MSG("getPath(): " << _pathfile);
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
	DEBUG_MSG("code fin de parse https = " << _sCode);
	// std::cout << "ode =" << _sCode << std::endl;
	if (_sCode == 200)
		parseParam();
}

void Request::parseChunkedBody(std::string::size_type pos, Client& cli)
{
	std::string::size_type end;
	std::string size_str;
	std::stringstream ss;
	size_t size = 0;

	while (1)
		{
			end = cli.getBuff().find("\r\n", pos);
			if (end == std::string::npos)
			{
				_sCode = 400;
				return ;
			}
			size_str = cli.getBuff().substr(pos, end - pos);
			ss << std::hex << size_str;
			ss >> size;
			if (ss.fail())
			{
				_sCode = 400;
				return ;
			}
			ss.clear();
			ss.str("");
			pos = end + 2;
			if (size == 0)
			{
				if (cli.getBuff().substr(pos, 2) != "\r\n")
				{
					_sCode = 400;
					return;
				}
				break;
			}
			if (pos + size + 2 > cli.getBuff().size())
			{
				_sCode = 400;
				return;
			}
			_body << cli.getBuff().substr(pos, size);
			if (cli.getBuff().substr(pos + size, 2) != "\r\n")
			{
				_sCode = 400;
				return ;
			}
			pos += size + 2;
		}
}


void Request::parseBody()
{
	std::string::size_type pos;
	// std::string::size_type end;
	Client& cli = _cli;
	// size_t size = 0;
	// std::string size_str;
	// std::stringstream ss;
	// perror("PErryr");
	pos = cli.getBuff().find("\r\n\r\n");
	if (_chunked != 1) // content-lenght body
	{
		if (pos + 4 < cli.getBuff().size())
			_body << cli.getBuff().substr(pos + 4);
	}
	else //chunked body
	{
		parseChunkedBody(pos + 4, cli);
		// pos += 4;
		// while (1)
		// {
		// 	end = cli.getBuff().find("\r\n", pos);
		// 	if (end == std::string::npos)
		// 	{
		// 		_sCode = 400;
		// 		return ;
		// 	}
		// 	size_str = cli.getBuff().substr(pos, end - pos);
		// 	ss << std::hex << size_str;
		// 	ss >> size;
		// 	if (ss.fail())
		// 	{
		// 		_sCode = 400;
		// 		return ;
		// 	}
		// 	ss.clear();
		// 	ss.str("");
		// 	pos = end + 2;
		// 	if (size == 0)
		// 	{
		// 		if (cli.getBuff().substr(pos, 2) != "\r\n")
		// 		{
		// 			_sCode = 400;
		// 			return;
		// 		}
		// 		break;
		// 	}
		// 	if (pos + size + 2 > cli.getBuff().size())
		// 	{
		// 		_sCode = 400;
		// 		return;
		// 	}
		// 	_body << cli.getBuff().substr(pos, size);
		// 	if (cli.getBuff().substr(pos + size, 2) != "\r\n")
		// 	{
		// 		_sCode = 400;
		// 		return ;
		// 	}
		// 	pos += size + 2;
		// }
	}

	DEBUG_MSG("body = " << _body.str());
	// _body << cli.getBuff().substr(pos + 4);
}
