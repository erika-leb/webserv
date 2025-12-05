#include "Request.hpp"
#include "all.hpp"

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

void Request::getPath(std::string &pathfile)
{
	Directive	directive;
	// int			j;

	// std::vector<LocationConfig> locs = _serv.getLocation();
	std::vector<std::string> arg;
	// j = -1;
	// std::cout << "coddddddddddeeeeee =" << _sCode << std::endl;
	// std::cout << "pathfile = " << pathfile << std::endl;
	getWriteLocation(pathfile);
	if (_locationIndex != -1)
		directive = getDirective("root", _locs[_locationIndex].getDir());
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
