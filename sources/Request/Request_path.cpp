#include "Request.hpp"
#include "all.hpp"

std::string Request::getExtention()
{
	size_t pos;

	pos = _pathfile.rfind(".");
	if (pos == std::string::npos)
		return ("");
	return (_pathfile.substr(pos));
}

const char *Request::getFileType()
{
	std::string ext;

	ext = getExtention();
	if (ext == "")
		return ("application/octet-stream");
	for (int i = 0; gType[i].ext != NULL; i++)
	{
		if (ext ==  gType[i].ext)
			return (gType[i].type);
	}
	return ("application/octet-stream");
}


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
	for (std::vector<LocationConfig>::size_type i = 0; i < _locs.size(); i++)
	{
		directive = getDirective("root", _locs[i].getDir());
		arg = directive.getArg();
		uri = _locs[i].getUri();
		// DEBUG_MSG("uri = " << uri);
		// DEBUG_MSG("pathbis = " << pathBis);
		if (pathBis.rfind(uri, 0) == 0)
		{
			// DEBUG_MSG("pathbis2 = " << pathBis);
			// DEBUG_MSG("uri size = " << uri.size() << " ; size = " << size);
			if (uri.size() > size)
			// if (arg[0].size() > size)
			{
				// DEBUG_MSG("uri2 = " << uri);
				size = uri.size();
				// size = arg[0].size();
				// DEBUG_MSG("arg0 = " << arg[0]);
				_locationIndex = i;
			}
		}
	}
}

void Request::getPath(std::string &pathfile)
{
	Directive	directive;

	// std::vector<LocationConfig> locs = _serv.getLocation();
	std::vector<std::string> arg;
	getWriteLocation(pathfile);
	if (_locationIndex != -1)
		directive = getDirective("root", _locs[_locationIndex].getDir());
	else
		directive = getDirective("root", _serv.getDir());
	arg = directive.getArg();
	// if (pathfile[0] != '/') //inutile ??
	// 	pathfile.insert(0, "/"); // inutile ??

	if (_sCode < 300)
		pathfile.insert(0, arg[0]);
}

std::string Request::getFile(std::string &pathfile, size_t *fileLength)
{
	std::ifstream fs(pathfile.c_str());
	std::string tmp, res;

	// getPath(pathfile);
	if (fs.is_open()) {
		while (std::getline(fs, tmp))
		{
			res += tmp;
			res += '\n';
		}
		fs.close();
		*fileLength = res.size();
	}
	else {
		DEBUG_MSG("ERROR: Couldn't open file [" << pathfile << "]");
		res.clear();
	}

	return (res);
}

// std::string Request::getFile(std::string &pathfile, size_t *fileLength)
// {
//     std::ifstream fs(pathfile.c_str(), std::ios::binary); // Ouverture en binaire

//     if (fs.is_open()) {
//         std::stringstream buffer;
//         buffer << fs.rdbuf(); // Copie directe du flux interne
//         fs.close();

//         std::string res = buffer.str();
//         *fileLength = res.size(); // Taille exacte en octets
//         return res;
//     }

//     DEBUG_MSG("ERROR: Couldn't open file [" << pathfile << "]");
//     *fileLength = 0;
//     return "";
// }

void Request::checkPath(std::string pathfile, size_t &eCode)
{
	struct stat	fileStat;

	if (_action == "POST")
		return;
	getPath(pathfile);
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
