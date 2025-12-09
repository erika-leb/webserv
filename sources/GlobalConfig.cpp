#include "GlobalConfig.hpp"

GlobalConfig::GlobalConfig(std::fstream &temp) : dir()
{
    std::string line;
    int serv = 0;

    while (1)
	{
        if (!(line.size() >= 6 && line.substr(0, 6) == "server"))
            std::getline(temp, line);
        if (!temp.eof() && temp.fail())
		    throw std::runtime_error("error while reading temp file" + static_cast<std::string>(strerror(errno)));
        else if (temp.eof())
            break ;
        else if (serv == 0 && line != "server")
        {
            dir.push_back(Directive(line));
        }
        else if (line == "server")
        {
            serv++;
            servs.push_back(ServerConfig(temp, this));
        }
        else
            throw std::runtime_error("error configuration file's syntaxe");
	}
    if (serv == 0)
		throw std::runtime_error("error configuration file should contain at least one server");
	checkAllowedDirective();
}

void GlobalConfig::checkAllowedDirective()
{
    for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
	{
		Directive &dir = *it;
		if (dir.getName() == "allow_methods")
    	    throw std::runtime_error("error in configuration file : allow_methods only auhtorized in locations");
		if (dir.getName() == "return")
    	    throw std::runtime_error("error in configuration file : redirection only auhtorized in locations");
		if (dir.getName() == "index")
    	    throw std::runtime_error("error in configuration file : index only auhtorized in locations");
	}
}

std::vector<Directive>& GlobalConfig::getDir()
{
	return (dir);
}

std::vector<ServerConfig>& GlobalConfig::getServ()
{
	return (servs);
}

void GlobalConfig::print_config()
{
    std::cout << "CONFIG" << std::endl;
    for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
    {
        Directive &dir = *it;
        dir.print_directive();
    }
    for (std::vector<ServerConfig>::iterator it = servs.begin(); it != servs.end(); ++it)
    {
        ServerConfig &serv = *it;
        serv.print_server();
    }
}

GlobalConfig::GlobalConfig(const GlobalConfig &src) : dir(src.dir), servs(src.servs)
{
}

GlobalConfig::~GlobalConfig()
{

}

GlobalConfig &GlobalConfig::operator=(const GlobalConfig &src)
{
    if (this != &src)
    {
        dir = src.dir;
        servs = src.servs;
    }
    return (*this);
}

GlobalConfig::GlobalConfig() : dir(), servs()
{
}
