#include "ServerConfig.hpp"

ServerConfig::ServerConfig(std::fstream &temp, GlobalConfig *gconf) : dir(), locs(), conf(gconf), _ip("127.0.0.1"), _port(8080)
{
    std::string line;
    int loc = 0;

    while (1)
	{
        if (!((line.size() >= 8 && line.substr(0, 8) == "location") || line == "server"))
            std::getline(temp, line);
        trim_line(line);
        if (!temp.eof() && temp.fail())
		    throw std::runtime_error("error while reading temp file" + static_cast<std::string>(strerror(errno)));
        else if (temp.eof() || line == "server")
        {
            break ;
        }
        else if (loc == 0 && line.substr(0, 8) != "location")
        {
            dir.push_back(Directive(line));
        }
        else if (line.substr(0, 8) == "location")
        {
            loc++;
            locs.push_back(LocationConfig(temp, line, this));
        }
    }
	checkListen();
	addGlobalDir();
	checkBasicDir();
}

void ServerConfig::checkBasicDir()
{
	if (isDirectivePresent("root", dir) == false)
		dir.push_back(Directive("root " + std::string(ROOT_DEFAULT)));
}

void ServerConfig::addGlobalDir()
{
	std::vector<Directive> globDir;
	int flag = 0;

	globDir = conf->getDir();
	for (std::vector<Directive>::iterator it = globDir.begin(); it != globDir.end(); ++it)
	{
		flag = 0;
		for (std::vector<Directive>::iterator ite = dir.begin(); ite != dir.end(); ++ite)
		{
			if (it->getName() == ite->getName())
			{
				flag = 1;
				break ;
			}
		}
		if (flag == 0)
			dir.push_back(*it);
	}
}

GlobalConfig *ServerConfig::getGlobConf()
{
	return (conf);
}


std::vector<Directive>& ServerConfig::getDir()
{
	return (dir);
}

int ServerConfig::getPort()
{
	return (_port);
}

std::string &ServerConfig::getIp()
{
	return (_ip);
}

std::vector<LocationConfig> &ServerConfig::getLocation()
{
	return (locs);
}

void ServerConfig::checkListen()
{
	std::vector<std::string> arg;

	for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
    {
        Directive &dir = *it;
        if (dir.getName() == "listen")
		{
			if (dir.getNbArg() != 1)
				throw std::runtime_error("error with configuration file : 'listen' directive should only have one argument");
			arg = dir.getArg();
			if (isValidListArg(arg[0]) == false)
				throw std::runtime_error("error with configuration file : 'listen' directive is not valid");
			return ;
		}
    }
	throw std::runtime_error("error with configuration file : server need one 'listen' directive");
}

bool ServerConfig::isValidListArg(std::string &s)
{
	if (isValidPort(s) ==  true)
		return (true);
	if (isValidHostPort(s) == true)
		return (true);
	return (false);
}

bool ServerConfig::isValidPort(std::string &s)
{
	int port;

	if (s.size() == 0)
		return false;
	for (std::string::size_type i = 0; i < s.size(); ++i)
	{
		if (!(std::isdigit(s[i])))
			return (false);
	}

	port = std::atoi(s.c_str());
	if (port < 1 || port > 65536)
		return (false);
	_port = port;
	return (true);
}

bool ServerConfig::isValidIPv4(std::string &s)
{
	int dots = 0;
	std::string block;
	int count = 0;
	int value;

	for (std::string::size_type i = 0; i < s.size(); ++i)
	{
		if (s[i] == '.')
			dots++;
	}
	if (dots != 3)
		return (false);
	for(std::string::size_type i = 0; i <= s.size(); ++i)
	{
		if (i == s.size() || s[i] == '.')
		{
			if (block.size() == 0)
				return (false);
			for (std::string::size_type j = 0; j < block.size(); ++j)
			{
                if (!std::isdigit(block[j]))
                    return (false);
			}
			value = std::atoi(block.c_str());
			if (value < 0 || value > 255)
				return (false);
			block = "";
			count++;
		}
		else
			block += s[i];
	}
	if (count == 4)
		_ip = s;
	return (count == 4);
}

bool ServerConfig::isValidHostPort(std::string &s)
{
	std::string host;
	std::string port;

	std::string::size_type pos = s.find(':');
	if (pos == std::string::npos)
		return (false);

	host = s.substr(0, pos);
	port = s.substr(pos + 1);

	if (port.size() == 0)
		return (false);
	if (isValidPort(port) == false)
		return (false);
	if (host == "*")
		return true;
	if (host == "0.0.0.0")
		return true;
	if (isValidIPv4(host))
		return true;
	return (false);
}


void ServerConfig::print_server()
{
    std::cout << "SERVER" << std::endl;
    for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
    {
        Directive &dir = *it;
        dir.print_directive();
    }
    for (std::vector<LocationConfig>::iterator it = locs.begin(); it != locs.end(); ++it)
    {
        LocationConfig &loc = *it;
        loc.print_location();
    }
}

ServerConfig::ServerConfig(const ServerConfig &src) : dir(src.dir), locs(src.locs), conf(src.conf), _ip(src._ip), _port(src._port)
{
}

ServerConfig::~ServerConfig()
{

}

ServerConfig &ServerConfig::operator=(const ServerConfig &src)
{
    if (this != &src)
    {
        dir = src.dir;
        locs = src.locs;
		conf = src.conf;
		_ip = src._ip;
		_port = src._port;
    }
    return (*this);
}
