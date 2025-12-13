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
	DEBUG_MSG("code fin de parse https = " << _sCode);
	// std::cout << "ode =" << _sCode << std::endl;
	if (_sCode == 200)
		parseParam();
}

void Request::parseBody()
{
	std::string::size_type pos;
	Client& cli = _cli;

	// perror("PErryr");
	pos = cli.getBuff().find("\r\n\r\n");
	if (pos + 4 < cli.getBuff().size())
		_body << cli.getBuff().substr(pos + 4);
	else
		_body << "";  // body vide
	DEBUG_MSG("body = " << _body.str());
	// _body << cli.getBuff().substr(pos + 4);
}
