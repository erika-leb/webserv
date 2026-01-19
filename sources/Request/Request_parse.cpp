#include "Request.hpp"
#include "all.hpp"

void Request::parseParam(void) //voir avec thibualt si besoin de faire le cas content-encoding
{


	std::string value;
	if (_reqParam.find("transfer-encoding") != _reqParam.end())
	{
		// DEBUG_MSG("ici code 6= " << _sCode);
		value = toLower(_reqParam["transfer-encoding"]);
		if (value ==  "chunked")
			_chunked = 1;
		else
		{
			DEBUG_MSG("ici code 8= " << _sCode);
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
		// DEBUG_MSG("_chunked = " << _chunked);
		if (_action == "POST" && _chunked == 0)
			_sCode = 411;
	}

	if (_reqParam.find("expect") != _reqParam.end())
	{
		_expect = 1;
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
		_sCode = 501;
	}
	std::getline(_rawHttp, _pathfile, ' ');
	remove_blank(_pathfile);

	// DEBUG_MSG("ici code = " << _sCode);
	std::string pathWithoutQuery(_pathfile);
	size_t end;
	if ( (end = _pathfile.find('?')) != std::string::npos )
		pathWithoutQuery = _pathfile.substr(0, end);
	// DEBUG_MSG("ici code 2= " << _sCode);
	if (_pathfile.empty())
		_sCode = 400;
	else
	{
		checkRedirAndMethod();
		checkPath(pathWithoutQuery, _sCode);
		getPath(_pathfile);
	}
	// DEBUG_MSG("ici code 3= " << _sCode);

	std::getline(_rawHttp, tmp);
	remove_blank(tmp);
	if (!tmp.empty())
	{
		if (tmp != "HTTP/1.1")
		{
			if (tmp == "HTTP/0.9" || tmp == "HTTP/2.0" || tmp == "HTTP/1.0")
			{
				_sCode = 505;
			}
			else
				_sCode = 400;
		}
	}
	else
		_sCode = 400;
	// DEBUG_MSG("ici code 4= " << _sCode);
	if (_sCode == 200)
		parseParam();
	// DEBUG_MSG("_pathfile = " << _pathfile);
	if (_sCode == 200 && _pathfile.find("..") != std::string::npos)
		_sCode = 403;
	// DEBUG_MSG("ici code fin *9= " << _sCode);
}

bool Request::parseChunkedBody(size_t pos, Client& cli)
{
	std::string::size_type end;
	std::string size_str;
	std::stringstream ss;
	size_t size = 0;

	while (1)
	{
        if (pos >= cli.getBuff().size())
            return false;
		end = cli.getBuff().find("\r\n", pos);
		if (end == std::string::npos)
			return (false);
		size_str = cli.getBuff().substr(pos, end - pos);
		ss << std::hex << size_str;
		ss >> size;
		if (ss.fail())
		{
			_sCode = 400;
			cli.clearRequestBuff(1, cli.getBuff().size());
			return (true);
		}
		ss.clear();
		ss.str("");
		pos = end + 2;
		if (size == 0)
		{
			if (pos + 2 > cli.getBuff().size())
				return (false);
			if (cli.getBuff().substr(pos, 2) != "\r\n")
			{
				_sCode = 400;
				cli.clearRequestBuff(1, cli.getBuff().size());
			}
			cli.clearRequestBuff(1, pos + 2);
			cli.getRequest()->checkLenght(pos + 2);
			return (true);
		}
		if (pos + size + 2 > cli.getBuff().size())
			return (false);
		if (cli.getBuff().substr(pos + size, 2) != "\r\n")
		{
			_sCode = 400;
			cli.clearRequestBuff(1, cli.getBuff().size());

			return (true);
		}
		_body << cli.getBuff().substr(pos, size);

		cli.clearRequestBuff(1, pos + size + 2);
		pos = 0;

	}
}

bool Request::parseBody()
{
	Client& cli = _cli;

	if (_chunked != 1) // content-lenght body
	{
		if (_contentLength <= cli.getBuff().size())
		{
			_body << cli.getBuff().substr(0, _contentLength);
			cli.clearRequestBuff(1, _contentLength);
			return (true);
		}
		else
			return (false);
			// DEBUG_MSG("PROBLEM");
	}
	else //chunked body
	{
		// DEBUG_MSG("do");
		bool complete;
		complete = parseChunkedBody(0, cli);

		return (complete);

	}
}
