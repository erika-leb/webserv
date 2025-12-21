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
		DEBUG_MSG("_chunked = " << _chunked);
		if (_action == "POST" && _chunked == 0)
			_sCode = 411;
	}

	perror("ici");
	if (_reqParam.find("expect") != _reqParam.end())
	{
		perror("la");
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
	DEBUG_MSG("rawhttp = " << _rawHttp.str());
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

	DEBUG_MSG("code 1 = " << _sCode);
	if (_pathfile.empty())
		_sCode = 400;
	else
	{
		checkRedirAndMethod();
		DEBUG_MSG("code 3 = " << _sCode);
		checkPath(pathWithoutQuery, _sCode);
		// checkPath(_pathfile, _sCode);
		getPath(_pathfile);
		DEBUG_MSG("getPath(): " << _pathfile);
	}
	DEBUG_MSG("code 2 = " << _sCode);
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

bool Request::parseChunkedBody(size_t pos, Client& cli)
{
	std::string::size_type end;
	std::string size_str;
	std::stringstream ss;
	size_t size = 0;

	std::cerr
  << "[CHUNK] buff.size=" << cli.getBuff().size()
  << " pos=" << pos
  << std::endl;
	while (1)
	{
        if (pos >= cli.getBuff().size())
            return false;
		end = cli.getBuff().find("\r\n", pos);
		if (end == std::string::npos)
		{
			// _sCode = 400;
			// return (-1);
			// return (std::string::npos);
			return (false);
		}
		size_str = cli.getBuff().substr(pos, end - pos);
		ss << std::hex << size_str;
		ss >> size;
		if (ss.fail())
		{
			_sCode = 400;
			cli.clearRequestBuff(1, cli.getBuff().size());
			// return (-1);
			// return (std::string::npos);
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
				

				// return (-1);
				// return (std::string::npos);
				// return (true);
			}
			cli.clearRequestBuff(1, pos + 2);
			cli.getRequest()->checkLenght(pos + 2);
			return (true);
		}
		if (pos + size + 2 > cli.getBuff().size())
		{
			// _sCode = 400;
			// return (std::string::npos);
			// return (-1);
			return (false);
		}
		if (cli.getBuff().substr(pos + size, 2) != "\r\n")
		{
			_sCode = 400;
			cli.clearRequestBuff(1, cli.getBuff().size());
			// return (-1);
			// return (std::string::npos);
			return (true);
		}
		_body << cli.getBuff().substr(pos, size);
		std::cerr << "[CHUNK] consumed " << size << " bytes" << std::endl;

		cli.clearRequestBuff(1, pos + size + 2);
		// pos += size + 2;
		pos = 0;

	}
}

// bool Request::parseChunkedBody(size_t pos, Client& cli)
// {
//     std::string::size_type end;
//     std::string size_str;
//     size_t size = 0;

//     std::cerr << "[CHUNK] buff.size=" << cli.getBuff().size()
//               << " pos=" << pos << std::endl;

//     while (1)
//     {
//         // garde-fou : si plus rien dans le buffer, il faut attendre plus de données
//         if (pos >= cli.getBuff().size())
//             return false;

//         end = cli.getBuff().find("\r\n", pos);
//         if (end == std::string::npos)
//             return false;

//         // extrait la taille hex du chunk
//         size_str = cli.getBuff().substr(pos, end - pos);

//         // parser la taille en hex avec un stringstream LOCAL
//         {
//             std::stringstream ss(size_str);
//             ss >> std::hex >> size;
//             if (ss.fail())
//             {
//                 _sCode = 400;
//                 // erreur de format : consommer le buffer et signaler la fin pour réponse
//                 cli.clearRequestBuff(1, cli.getBuff().size());
//                 return true;
//             }
//         }

//         pos = end + 2; // position du début des données du chunk

//         // chunk terminal
//         if (size == 0)
//         {
//             // on attend les 2 octets terminaux "\r\n"
//             if (pos + 2 > cli.getBuff().size())
//                 return false;

//             // **vérifier AVANT de vider le buffer**
//             if (cli.getBuff().substr(pos, 2) != "\r\n")
//             {
//                 _sCode = 400;
//                 cli.clearRequestBuff(1, cli.getBuff().size());
//                 return true;
//             }

//             // consommer "0\r\n\r\n"
//             cli.clearRequestBuff(1, pos + 2);
//             cli.getRequest()->checkLenght(pos + 2);
//             return true;
//         }

//         // vérifier qu'on a reçu tout le chunk + CRLF final
//         if (pos + size + 2 > cli.getBuff().size())
//             return false;

//         // vérifier CRLF après le chunk
//         if (cli.getBuff().substr(pos + size, 2) != "\r\n")
//         {
//             _sCode = 400;
//             cli.clearRequestBuff(1, cli.getBuff().size());
//             return true;
//         }

//         // consommer les données du chunk
//         _body << cli.getBuff().substr(pos, size);
//         std::cerr << "[CHUNK] consumed " << size << " bytes" << std::endl;

//         // effacer du buffer tout ce qu'on a consommé (taille + CRLF)
//         cli.clearRequestBuff(1, pos + size + 2);

//         // repartir du début du buffer restant
//         pos = 0;
//     }
// }



bool Request::parseBody()
{
	// std::string::size_type pos;
	// std::string::size_type end;
	Client& cli = _cli;
	// size_t consumed;

	DEBUG_MSG("chunk = " << _chunked);
	if (_chunked != 1) // content-lenght body
	{
		// if (pos + 4 < cli.getBuff().size())
		// 	_body << cli.getBuff().substr(pos + 4);
		DEBUG_MSG("you");
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
		// parseChunkedBody(pos + 4, cli);
		DEBUG_MSG("do");
		bool complete;
		complete = parseChunkedBody(0, cli);
	    std::cout << "Chunked check: " << cli.getBuff().size() << " bytes in buffer, Complete: " << complete << std::endl;
		// return (parseChunkedBody(0, cli));
		std::cerr
  		<< "[BODY] after parse: buff.size="
  		<< cli.getBuff().size()
  		<< std::endl;

		return (complete);
		// consumed = parseChunkedBody(0, cli);
		// if (consumed == std::string::npos)
		// 	cli.clearRequestBuff(1, cli.getBuff().size());
		// else
		// 	cli.clearRequestBuff(1, consumed);

	}

	// DEBUG_MSG("body = " << _body.str());
	// _body << cli.getBuff().substr(pos + 4);
}
