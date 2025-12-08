#include "Request.hpp"
#include "all.hpp"


void Request::setErrorPath()
{
	std::vector<Directive> dirs;
	std::string name;
	std::vector<std::string> arg;
	Directive root;

	if (_locationIndex == -1)
	{
		dirs = _serv.getDir();
		root = getDirective("root", dirs);
	}
	else
	{
		dirs = _locs[_locationIndex].getDir();
		root = getDirective("root", dirs);
	}
	// DEBUG_MSG(" _locationIndex = " << _locationIndex);
	for (std::vector<Directive>::size_type i = 0; i < dirs.size(); i++)
	{
		name = dirs[i].getName();
		arg = dirs[i].getArg();
		if (name == "error_page" && arg[0] == "500")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[500] = root.getArg()[0] + arg[1];
				else
					_errorPath[500] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "400")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[400] = root.getArg()[0] + arg[1];
				else
					_errorPath[400] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "403")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[403] = root.getArg()[0] + arg[1];
				else
					_errorPath[403] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "404")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
				{
					perror("lady");
					DEBUG_MSG(root.getArg()[0] << "| et |" <<arg[1]);
					_errorPath[404] = root.getArg()[0] + arg[1];
				}
				else
					_errorPath[404] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "405")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[405] = root.getArg()[0] + arg[1];
				else
					_errorPath[405] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "411")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[411] = root.getArg()[0] + arg[1];
				else
					_errorPath[411] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		if (name == "error_page" && arg[0] == "501")
		{
			if (access(arg[1].c_str(), F_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[501] = root.getArg()[0] + arg[1];
				else
					_errorPath[501] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
	}
	if (_locationIndex == -1 && _errorPath.find(405) == _errorPath.end())
		_errorPath[405] = ROOT_STR + ERROR_405; // chemin entier
	if (_locationIndex == -1 && _errorPath.find(400) == _errorPath.end())
		_errorPath[400] = ROOT_STR + ERROR_400;
	if (_locationIndex == -1 && _errorPath.find(403) == _errorPath.end())
		_errorPath[403] = ROOT_STR + ERROR_403;
	if (_locationIndex == -1 && _errorPath.find(404) == _errorPath.end())
		_errorPath[404] = ROOT_STR + ERROR_404;
	if (_locationIndex == -1 && _errorPath.find(411) == _errorPath.end())
		_errorPath[411] = ROOT_STR + ERROR_411;
	if (_locationIndex == -1 && _errorPath.find(500) == _errorPath.end())
		_errorPath[500] = ROOT_STR + ERROR_500;
	if (_locationIndex == -1 && _errorPath.find(501) == _errorPath.end())
		_errorPath[501] = ROOT_STR + ERROR_501;
	DEBUG_MSG("400 = " << _errorPath[400]);
	DEBUG_MSG("403 = " << _errorPath[403]);
	DEBUG_MSG("404 = " << _errorPath[404]);
	DEBUG_MSG("405 = " << _errorPath[405]);
	DEBUG_MSG("500 = " << _errorPath[500]);
}


/* This function can be reduce with a hash table */
std::string Request::ifError(std::string &path, std::string &con, int sCode)
{
	std::string str;
	switch (sCode)
	{
	case 201:
		// path = newly created ressource
		str = " Created";
		break ;
	case 204:
		// no path
		str = " No content";
		break ;
	case 301:
		path = ROOT_STR + REDIR_301;
		str = " Moved Permanently";
		break ;
	case 302:
		path = ROOT_STR + REDIR_302;
		str = " Found";
		break ;
	case 307:
		path = ROOT_STR + REDIR_307;
		str = " Temporary Redirect";
		break ;
	case 308:
		path = ROOT_STR + REDIR_308;
		str = " Permanent Redirect";
		break ;
	case 400:
		path = _errorPath[400];
		str = " Bad request";
		con = "close";
		break ;
	case 403:
		path = _errorPath[403];
		str = " Forbidden";
		break ;
	case 404:
		path = _errorPath[404];
		str = " Not found";
		break ;
	case 405:
		path = _errorPath[405];
		str = " Method not allowed";
		break ; // Connection normally keep-alive
	case 411:
		path = _errorPath[411];
		str = " Length required";
		break ; // Connection normally keep-alive
	case 500:
		path = _errorPath[500];
		str = " Internal server error";
		con = "close";
		break ;
	case 501:
		path = _errorPath[501];
		str = " Not implemented";
	default:
		str = " Ok";
		con = "keep-alive";
		break ;
	}
	return (str);
}
