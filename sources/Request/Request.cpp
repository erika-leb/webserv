#include "Request.hpp"
#include "all.hpp"

// NB = pas trop de protection sur les getline

// void Request::getWriteLocation(std::string &pathfile)
// {
// 	Directive	directive;
// 	size_t		size;
// 	std::vector<std::string> arg;
// 	std::string uri;
// 	size = 0;
// 	DEBUG_MSG("Paht avant de chercher la loc " << pathfile);
// 	for (std::vector<LocationConfig>::size_type i = 0; i < _locs.size(); i++)
// 	{
// 		directive = getDirective("root", _locs[i].getDir());
// 		arg = directive.getArg();
// 		uri = _locs[i].getUri();
// 		DEBUG_MSG("uri " << _locs[i].getUri() << " et i =" << i);
// 		if (pathfile.rfind(uri, 0) == 0)
// 		{
// 			// DEBUG_MSG("uri " << _locs[i].getUri() << "P");
// 			// DEBUG_MSG("arg[0].size() " << arg[0].size());
// 			// DEBUG_MSG("(*size) " << size);
// 			if (uri.size() > size)
// 			// if (arg[0].size() > size)
// 			{
// 				// perror("omg");
// 				size = arg[0].size();
// 				_locationIndex = i;
// 			}
// 		}
// 	}
// 	DEBUG_MSG("_index final " << _locationIndex);
// }

void Request::getWriteLocation(std::string &pathfile)
{
	Directive	directive;
	std::vector<std::string> arg;
	std::string uri;
	size_t size = 0;
	std::string pathBis;

	if (pathfile[pathfile.size() - 1] != '/')
		pathBis = pathfile + '/';
	else
		pathBis = pathfile;
	// DEBUG_MSG("Paht " << pathfile);
	for (std::vector<LocationConfig>::size_type i = 0; i < _locs.size(); i++)
	{
		directive = getDirective("root", _locs[i].getDir());
		arg = directive.getArg();
		uri = _locs[i].getUri();
		// DEBUG_MSG("uri " << _locs[i].getUri()<< " et i =" << i);
		if (pathBis.rfind(uri, 0) == 0)
		{
			// DEBUG_MSG("uri " << _locs[i].getUri()<< "P");
			// DEBUG_MSG("arg[0].size() " << arg[0].size());
			// DEBUG_MSG("(*size) " << size);
			if (uri.size() > size)
			// if (arg[0].size() > size)
			{
				// perror("omg");
				size = arg[0].size();
				_locationIndex = i;
			}
		}
	}
	DEBUG_MSG("j final " << _locationIndex);
}

void Request::checkRedirAndMethod()
{
	int			j;
	Directive	dir;
	int			flag;
	int			code;

	// std::vector<LocationConfig> locs = _serv.getLocation();
	j = -1;
	std::vector<std::string> arg;
	flag = 0;
	getWriteLocation(_pathfile);
	// std::cout << "j = " << j << std::endl;
	if (j != -1)
	{
		for (std::vector<Directive>::size_type i = 0; i < _locs[j].getDir().size(); i++)
		{
			dir = _locs[j].getDir()[i];
			if (dir.getName() == "allow_methods")
			{
				flag = 1;
				// perror("exper");
				arg = dir.getArg();
				for (std::vector<std::string>::size_type k = 0; k < arg.size(); k++)
				{
					if (arg[k] == _action)
						flag = 0;
				}
			}
			if (dir.getName() == "return")
			{
				// perror("riment");
				arg = dir.getArg();
				code = std::atoi(arg[0].c_str());
				_sCode = code;
				_location = arg[1];
				return ;
			}
		}
	}
	if (flag == 1) // methode pas autorisee
		_sCode = 405;
}

void Request::getPath(std::string &pathfile)
{
	Directive	directive;
	int			j;

	// std::vector<LocationConfig> locs = _serv.getLocation();
	std::vector<std::string> arg;
	j = -1;
	// std::cout << "coddddddddddeeeeee =" << _sCode << std::endl;
	// std::cout << "pathfile = " << pathfile << std::endl;
	getWriteLocation(pathfile);
	if (j != -1)
		directive = getDirective("root", _locs[j].getDir());
	else
		directive = getDirective("root", _serv.getDir());
	arg = directive.getArg();
	// if (pathfile[0] != '/') //inutile ??
	// 	pathfile.insert(0, "/"); // inutile ??
	// std::cout << "codeeeeee =" << _sCode << std::endl;
	if (_sCode < 300)
		pathfile.insert(0, arg[0]);
	// DEBUG_MSG("400 = " << _errorPath[400]);
	// DEBUG_MSG("403 = " << _errorPath[403]);
	// DEBUG_MSG("404 = " << _errorPath[404]);
	// DEBUG_MSG("405 = " << _errorPath[405]);
	// DEBUG_MSG("500 = " << _errorPath[500]);
}

// static std::string getFile( std::string &pathfile, size_t* fileLength ) {
std::string Request::getFile(std::string &pathfile, size_t *fileLength)
{
	std::fstream fs;
	std::string tmp, res;
	DEBUG_MSG("first path to get " << pathfile);
	getPath(pathfile);
	fs.open(pathfile.c_str(), std::ios::in);
	if (fs.is_open())
	{
		while (std::getline(fs, tmp))
		{
			res += tmp;
			tmp.clear();
		}
		fs.close();
		*fileLength = res.size();
	}
	else
	{
		DEBUG_MSG("ERROR: Couldn't open file [" << pathfile << "]");
		res.clear();
	}
	DEBUG_MSG("after first path to get " << pathfile);
	return (res);
}

void Request::checkIndex()
{
	Directive	dir;
	Directive	dir1;
	std::string indexPath;
	struct stat	fileStat;

	// std::cout << "i index = " << _locationIndex << std::endl;
	// std::cout << "path = " << _pathfile << std::endl;
	std::vector<Directive> dirs;
	if (_locationIndex != -1)
		dirs = _locs[_locationIndex].getDir();
	// else
		// dirs = _serv.getDir();
	DEBUG_MSG("GAGA");
	if (isDirectivePresent("index", dirs) == true)
	{
		DEBUG_MSG("LADY");
		dir = getDirective("index", dirs);
		dir1 = getDirective("root", dirs);
		DEBUG_MSG("URI =" << _locs[_locationIndex].getUri());
		indexPath = _locs[_locationIndex].getUri() + dir.getArg()[0];
		// if (_locs[_locationIndex].getUri() == "/")
		// 	indexPath = dir1.getArg()[0] + dir.getArg()[0];
		// else
		// 	indexPath = dir1.getArg()[0] + _locs[_locationIndex].getUri() + dir.getArg()[0];
		indexPath = dir1.getArg()[0] + _locs[_locationIndex].getUri() + dir.getArg()[0];
		if (stat(indexPath .c_str(), &fileStat) >= 0 && (S_ISREG(fileStat.st_mode))) // le fichier html est ok
		{
			_pathfile = _locs[_locationIndex].getUri() + dir.getArg()[0]; // PATHFILE FINAL
			std::cout << "chemin index = " << indexPath << std::endl;
		}
		// indexPath = _locs[_locationIndex].getUri() + dir.getArg()[0]; // PATHFILE FINAL

	}
	// on regarde s'il y a un index et qui fonctionne
	// sinon on va activer autoindex:
	// si on on genere un htlm
	// sinon erreur 403
}

void Request::checkPath(std::string pathfile, size_t &eCode)
{
	struct stat	fileStat;

	getPath(pathfile);
	DEBUG_MSG("after first getPaht = " << pathfile);
	if (_sCode != 200)
		return ;
	if (stat(pathfile.c_str(), &fileStat) < 0)
	{
		eCode = 404;
		DEBUG_MSG("Path does not exist: " << pathfile);
		return ;
	}
	if (S_ISREG(fileStat.st_mode)) // c'est un fichier
	{
		DEBUG_MSG("il s'agit d'un fichier");
	}
	else if (S_ISDIR(fileStat.st_mode)) // c'est un dossier
	{
		DEBUG_MSG("il s'agit d'un dossier");
		// if ((pathfile.c_str(), F_OK) < 0) {
		checkIndex();
		// eCode = 404;
		// }
	}
	else // c'est un autre type de fichier
	{
		eCode = 403; // Accès non autorisé à ce type
		DEBUG_MSG("Path is neither file nor directory: " << pathfile);
	}
	if ((pathfile.find("/errors/")) != std::string::npos)
	{
		eCode = 403;
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
	if (_pathfile.empty())
		_sCode = 400; // ici plutot
	// else if (IsRedir() == true)
	// { }
	// else if (IsMethodAllowed() == false)
	// 	_sCode = 405;
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
	std::cout << "ode =" << _sCode << std::endl;
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
	_sCode = 201;
	fGet();
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
	std::cerr << "message envopye = " << mess.str() << std::endl;
	_cli.setSendBuff(mess.str());
	if (_connection == "keep-alive")
		_cli.setCon(true);
	else
		_cli.setCon(false);
	return (_pathfile);
}
