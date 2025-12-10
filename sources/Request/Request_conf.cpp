#include "Request.hpp"
#include "all.hpp"

void Request::checkRedirAndMethod()
{
	Directive	dir;
	int			flag;
	int			code;

	// std::vector<LocationConfig> locs = _serv.getLocation();
	std::vector<std::string> arg;
	flag = 0;
	getWriteLocation(_pathfile);
	if (_locationIndex != -1)
	{
		for (std::vector<Directive>::size_type i = 0; i < _locs[_locationIndex].getDir().size(); i++)
		{
			dir = _locs[_locationIndex].getDir()[i];
			if (dir.getName() == "allow_methods")
			{
				flag = 1;
				arg = dir.getArg();
				for (std::vector<std::string>::size_type k = 0; k < arg.size(); k++)
				{
					if (arg[k] == _action)
						flag = 0;
				}
			}
			if (dir.getName() == "return")
			{
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

void Request::generateHtml(std::string uri, std::string path)
{
	std::stringstream body, response;
	std::string name;
	struct dirent *entry;
	std::cout << "PATH HTML = " << path << std::endl;
	DIR *dir = opendir(path.c_str());

	if (!dir)
	{
		body << "<!DOCTYPE html>\n<html><body>\n";
        body << "<p>Cannot open directory</p>\n";
	}
	else
	{
		body << "<!DOCTYPE html>\n<html>\n<head>\n";
    	body << "<meta charset=\"UTF-8\">\n";
		body << "<title>Index of " << uri << "</title>\n";
    	body << "</head>\n<body>\n";
    	body << "<h1>Index of " << uri << "</h1>\n";
    	body << "<ul>\n";

		while ((entry = readdir(dir)) != NULL)
		{
			name = entry->d_name;
			if (name == "." || name == "..")
				continue;
			body << "<li><a href=\"";
			body << uri;
            if (uri.size() > 0 && uri[uri.size() - 1] != '/')
                body << "/";
			body << name;
			if (entry->d_type == DT_DIR)
				body << "/";
			body << "\">" << name;
    	    if (entry->d_type == DT_DIR)
				body << "/";
			body << "</a></li>\n";
		}
		closedir(dir);
	}
	body << "</ul>\n</body>\n</html>\n";

	response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Content-Length: " << body.str().size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";  // séparation entête / body
    response << body.str();

	_htmlList << response.str();
}

void Request::checkIndex()
{
	Directive	dir;
	Directive	dir1;
	std::string indexPath;
	struct stat	fileStat;

	std::vector<Directive> dirs;
	if (_locationIndex != -1)
		dirs = _locs[_locationIndex].getDir();
	// else
		// dirs = _serv.getDir();
	dir1 = getDirective("root", dirs);
	if (isDirectivePresent("index", dirs) == true)
	{
		dir = getDirective("index", dirs);
		// dir1 = getDirective("root", dirs);
		indexPath = _locs[_locationIndex].getUri() + dir.getArg()[0];
		// if (_locs[_locationIndex].getUri() == "/")
		// 	indexPath = dir1.getArg()[0] + dir.getArg()[0];
		// else
		// 	indexPath = dir1.getArg()[0] + _locs[_locationIndex].getUri() + dir.getArg()[0];
		indexPath = dir1.getArg()[0] + _locs[_locationIndex].getUri() + dir.getArg()[0];
		if (stat(indexPath .c_str(), &fileStat) >= 0 && (S_ISREG(fileStat.st_mode))) // le fichier html est ok donc on sort
		{
			_pathfile = _locs[_locationIndex].getUri() + dir.getArg()[0]; // PATHFILE FINAL
			return;
		}
		// indexPath = _locs[_locationIndex].getUri() + dir.getArg()[0]; // PATHFILE FINAL
	}
	if (isDirectivePresent("autoindex", dirs) == true)
	{
		dir = getDirective("autoindex", dirs);
		if (dir.getArg()[0] == "off")
			_sCode = 403;
		else
		{
			indexPath = dir1.getArg()[0] + _locs[_locationIndex].getUri();
			generateHtml(_locs[_locationIndex].getUri(), indexPath);
		}
	}
	else
		_sCode = 403;
	// on regarde s'il y a un index et qui fonctionne
	// sinon on va activer autoindex:
	// si on on genere un htlm
	// sinon erreur 403
}
