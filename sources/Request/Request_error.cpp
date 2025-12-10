#include "Request.hpp"
#include "all.hpp"

static void setStatusInfo( int code, StatusInfo& tmp) {
	std::string str, con("");
	std::string defaultPath(ROOT_STR);

	switch (code)
	{
	case 200:
		str = " Ok";
		con = "keep-alive";
		break;
	case 201:
		// path = newly created ressource
		str = " Created";
		break ;
	case 204:
		// no path
		str = " No content";
		break ;
	case 301:
		defaultPath += REDIR_301;
		str = " Moved Permanently";
		break ;
	case 302:
		defaultPath += REDIR_302;
		str = " Found";
		break ;
	case 307:
		defaultPath += REDIR_307;
		str = " Temporary Redirect";
		break ;
	case 308:
		defaultPath += REDIR_308;
		str = " Permanent Redirect";
		break ;
	case 400:
		defaultPath += ERROR_400;
		str = " Bad request";
		con = "close";
		break ;
	case 403:
		defaultPath += ERROR_403;
		str = " Forbidden";
		break ;
	case 404:
		defaultPath += ERROR_404;
		str = " Not found";
		break ;
	case 405:
		defaultPath += ERROR_405;
		str = " Method not allowed";
		break ; // Connection normally keep-alive
	case 500:
		defaultPath += ERROR_500;
		str = " Internal server error";
		con = "close";
		break ;
	default:
		str = "";
		con = "";
		break ;
	}

	tmp.message = str; tmp.conn = con;
	if (defaultPath != ROOT_STR)
		tmp.path = defaultPath;
	else
		tmp.path = "";
}

void Request::setErrorPath()
{
	std::vector<Directive> dirs;
	std::string name;
	std::vector<std::string> arg;
	Directive root;
	StatusInfo tmp;

	for (size_t i=200; i <= 500; i++) {
		setStatusInfo(i, tmp);
		_errorPath.insert(std::pair<int, StatusInfo>(i, tmp));
	}

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

	for (std::vector<Directive>::size_type i = 0; i < dirs.size(); i++)
	{
		name = dirs[i].getName();
		arg = dirs[i].getArg();

		std::istringstream iss(arg[0]);
		int code;

		iss >> code;

		// Modular statement of the painfull code under
		/*
		if (name == "error_page" && (code >= 400 && code <= 500)) {
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
			{
				if (_locationIndex == -1)
					_errorPath[code] = root.getArg()[0] + arg[1];
				else
					_errorPath[code] = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
			}
		}
		*/

		if (name == "error_page" && (code >= 400 && code <= 500)) {
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
			{
				if (_locationIndex == -1)
					tmp.path = root.getArg()[0] + arg[1];
				else
					tmp.path = root.getArg()[0]
						+ _locs[_locationIndex].getUri() + arg[1];
				setStatusInfo(code, tmp);
				_errorPath[code] = tmp;
			}
		}

		/*
		if (name == "error_page" && arg[0] == "500")
		{
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
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
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
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
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
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
			if (access(arg[1].c_str(), F_OK | R_OK) < 0)
			{
				if (_locationIndex == -1)
				{
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
		*/
	}
}


/* This function can be reduce with a hash table */
std::string Request::ifError(std::string &path, std::string &con, int sCode)
{
	StatusInfo tmp = _errorPath[sCode];
	con = tmp.conn;
	path = tmp.path;
	return (tmp.message);
}
