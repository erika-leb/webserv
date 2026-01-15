#include "Request.hpp"
#include "all.hpp"

// NB = pas trop de protection sur les getline

void Request::fGet(void) {
	DEBUG_MSG("GET request");
	// DEBUG_MSG("conn 2= " << _connection );
	// DEBUG_MSG("path avant iferror : " << _pathfile);
	ifError(_pathfile, _connection, _sCode);
	// DEBUG_MSG("conn 1 = " << _connection );
	// DEBUG_MSG("path apres iferror : " << _pathfile);
	_file = getFile(_pathfile, &_fileLength);
	// DEBUG_MSG("conn 5 = " << _connection );
	DEBUG_MSG("path apres getfile : " << _pathfile);
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
	if (stat(_pathfile.c_str(), &st) == -1) // the file does not exist
	{
		perror("sdanse");
		if (errno != ENOENT) // file not found
		{
			perror("cors");
			_sCode = 403;
			fGet();
			return (1);
		}
		std::string::size_type pos = _pathfile.find_last_of('/'); // check if the parent exist
		if (pos == std::string::npos)
		{
			perror("brule");
			_sCode = 403;
			return (1);
		}

		std::string parent = _pathfile.substr(0, pos);
		// parent.insert(0, directive.getArg()[0]);
		DEBUG_MSG("parent = " << parent);
		if (access(parent.c_str(), W_OK) != 0) // check the right of writing in the parent
		{
			perror("au");
			_sCode = 403;
			fGet();
			return (1);
		}
	}
	else // file does exist
	{
		perror("singe");
		if (S_ISDIR(st.st_mode)) // the file is a directory
		{
			DEBUG_MSG("Path is a directory, cannot POST");
            _sCode = 403; // Ou 409 Conflict, mais 403 est standard ici
            fGet();
            return (1);
		}
		if (access(_pathfile.c_str(), W_OK) != 0) // we cannot write in the file
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

	// DEBUG_MSG("path avant check = " << _pathfile );

	if (checkPostPath(directive) == 1)
		return ;

	// _pathfile.insert(0, directive.getArg()[0]);
	// DEBUG_MSG("path avantcreation fichier post = " << _pathfile );
	upload.open(_pathfile.c_str(), std::ios::out | std::ios::trunc);
	// upload.open(_pathfile.c_str(), std::ios::out | std::ios::trunc);
	if (!upload.is_open())
	{
		perror("conemara");
		_sCode = 500;
		fGet();
		return;
	}
	// DEBUG_MSG("body = " << _body.str());
	upload << _body.str();
	_sCode = 201;
	upload.close();
	DEBUG_MSG("code sans return fin post = " << _sCode);
	// fGet(); // pourquoi il y a un fGet ? il faut l'enlever je crois
}

void Request::handleAction(std::string action)
{
	int	i;

	// DEBUG_MSG("conn = " << _connection );
	DEBUG_MSG("path debut action =" << _pathfile);
	DEBUG_MSG("action =" << action);
	DEBUG_MSG("code =" << _sCode);
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
	DEBUG_MSG("path = " << _pathfile );
	std::ostringstream mess;
	const char *type;

	mess << "HTTP/1.1"
			<< " " << _sCode << ifError(_pathfile, _connection,
				_sCode) << ENDLINE;
	mess << "Date: " << date(HTTP) << ENDLINE;
	mess << "Server: " << _serv.getIp() << ":" << _serv.getPort() << ENDLINE;
	mess << "Connection: " << _connection << ENDLINE;

	if (_sCode > 300 && _sCode < 400)
	{
		mess << "Location: " << _location << ENDLINE;
	}

	type = getFileType();
	mess << "Content-Type: "
				// << "text/html" << ENDLINE; // Modify according to file
				<< type << ENDLINE; // Modify according to file

	if (_sCode != 204 && _sCode != 201)
	{

		mess << "Content-Length: " << _fileLength << ENDLINE;
		mess << ENDLINE;
		mess << _file;
	}
	else
	{
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
