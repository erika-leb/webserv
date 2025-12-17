#include "Request.hpp"
#include "all.hpp"

// NB = pas trop de protection sur les getline

void Request::fGet(void) {
	DEBUG_MSG("GET request");
	ifError(_pathfile, _connection, _sCode);
	_file = getFile(_pathfile, &_fileLength);
	if (_file.empty())
	{
		_sCode = 403;
		ifError(_pathfile, _connection, _sCode);
		_file = getFile(_pathfile, &_fileLength);
	}
}

void Request::fDelete(void) {
	DEBUG_MSG("DELETE request");
	// getPath(_pathfile);
	if (access(_pathfile.c_str(), W_OK) == 0) {
		std::remove(_pathfile.c_str());
		_sCode = 204;
		return ;
	}
	_sCode = 403;
	fGet();
}

int Request::checkPostPath(Directive &directive)
{
	struct stat st;

	(void) directive;
	if (stat(_pathfile.c_str(), &st) == -1)
	{
		if (errno != ENOENT)
		{
			perror("cors");
			_sCode = 403;
			fGet();
			return (1);
		}
		std::string::size_type pos = _pathfile.find_last_of('/');
		if (pos == std::string::npos)
		{
			perror("brule");
			_sCode = 403;
			return (1);
		}

		std::string parent = _pathfile.substr(0, pos);
		// parent.insert(0, directive.getArg()[0]);
		DEBUG_MSG("parent = " << parent);
		if (access(parent.c_str(), W_OK) != 0)
		{
			perror("au");
			_sCode = 403;
			fGet();
			return (1);
		}
	}
	else
	{
		if (access(_pathfile.c_str(), W_OK) != 0)
		{
			perror("sang");
			_sCode = 403;
			fGet();
			return (1);
		}
	}
	return (0);
}


void Request::fPost(void)
{
	DEBUG_MSG("POST request");
	std::fstream upload;
	Directive directive;

	// processer l'info = cgi, uplaod si autoriser par la locaion

	if (_locationIndex != -1)
		directive = getDirective("root", _locs[_locationIndex].getDir());
	else
		directive = getDirective("root", _serv.getDir());

	if (checkPostPath(directive) == 1)
		return ;

	// _pathfile.insert(0, directive.getArg()[0]);
	DEBUG_MSG("path avantcreation fichier post = " << _pathfile );
	upload.open(_pathfile.c_str(), std::ios::out | std::ios::trunc);
	// upload.open(_pathfile.c_str(), std::ios::out | std::ios::trunc);
	if (!upload.is_open())
	{
		// perror("conemara");
		_sCode = 500;
		fGet();
		return;
	}
	DEBUG_MSG("body = " << _body.str());
	upload << _body.str();
	_sCode = 201;
	upload.close();
	DEBUG_MSG("code sans return fin post = " << _sCode);
	// fGet(); // pourquoi il y a un fGet ? il faut l'enlever je crois
}

void Request::handleAction(std::string action)
{
	int	i;

	std::string check[3] = {"GET", "POST", "DELETE"};
	void (Request::*f[3])(void) = {&Request::fGet, &Request::fPost,
		&Request::fDelete};
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
	DEBUG_MSG("code final =" << _sCode);
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
	if (_sCode != 204 && _sCode != 201)
	{
		mess << "Content-Type: "
				<< "text/html" << ENDLINE; // Modify according to file
		mess << "Content-Length: " << _fileLength << ENDLINE;
		mess << ENDLINE;
		mess << _file;
	}
	else
	{
		mess << "Content-Type: "
				<< "text/html" << ENDLINE; // Modify according to file
		mess << "Content-Length: " << 0 << ENDLINE;
		mess << ENDLINE;
	}
	if (_htmlList.str() == "")
	{
		std::cerr << "message envoye = " << mess.str() << std::endl;
		_cli.setSendBuff(mess.str());
	}
	else
	{
		std::cerr << "message envoye = " << _htmlList.str() << std::endl;
		_cli.setSendBuff(_htmlList.str());
	}

	if (_connection == "keep-alive")
		_cli.setCon(true);
	else
		_cli.setCon(false);
	DEBUG_MSG("path a fin de message = " << _pathfile);
	return (_pathfile);
}
