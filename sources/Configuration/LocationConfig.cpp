#include "LocationConfig.hpp"

LocationConfig::LocationConfig(std::fstream &temp, std::string &line, ServerConfig *sconf) : dir(), uri(), conf(sconf)
{
    get_uri(line);

    while (1)
	{
        std::getline(temp, line);
        trim_line(line);
        if (!temp.eof() && temp.fail())
		    throw std::runtime_error("error while reading temp file" + static_cast<std::string>(strerror(errno)));
        if (temp.eof() || (line.size() >= 8 && line.substr(0, 8) == "location") || (line.size() >= 6 && line == "server"))
        {
            break ;
        }
        dir.push_back(Directive(line));
	}
	addOtherDir();
	checkBasicDir();
}

void LocationConfig::checkBasicDir()
{
	if (isDirectivePresent("root", dir) == false) //add a default root if necessary
	{
		std::string tmp = std::string("root ") + ROOT;
		dir.push_back(Directive(tmp));
	}
	if (uri.find("..") != std::string::npos)
		throw std::runtime_error("error configuration file's uri name");
	if (uri.find("~") != std::string::npos)
		throw std::runtime_error("error configuration file's uri name");
}

void LocationConfig::addOtherDir()
{
	std::vector<Directive> servDir;
	std::vector<Directive> globDir;
	int flag;

	servDir = conf->getDir();
	for (std::vector<Directive>::iterator it = servDir.begin(); it != servDir.end(); ++it)
	{
		flag = 0;
		for (std::vector<Directive>::iterator ite = dir.begin(); ite != dir.end(); ++ite)
		{
			if (it->getName() == ite->getName() && it->getName() != "error_page")
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0)
			dir.push_back(*it);
	}

	globDir = conf->getGlobConf()->getDir();
	for (std::vector<Directive>::iterator it = globDir.begin(); it != globDir.end(); ++it)
	{
		flag = 0;
		for (std::vector<Directive>::iterator ite = dir.begin(); ite != dir.end(); ++ite)
		{
			if (it->getName() == ite->getName())
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0)
			dir.push_back(*it);
	}
}

void LocationConfig::get_uri(std::string &line)
{
    size_t i = 0;

    line = line.substr(8);
    trim_line(line);

    while (i < line.size() && !std::isspace(line[i]))
        i++;
    uri = line.substr(0, i);
    line = line.substr(i);
    trim_line(line);
    for (size_t j = 0; j < line.size(); j++)
    {
        if (!std::isspace(line[j]))
		    throw std::runtime_error("error in configuration file's syntax (location)");
    }
    if (uri == "" || uri[0] != '/')
		throw std::runtime_error("error in configuration file's syntax (location)");
	if ((uri[uri.size() - 1]) != '/')
		uri = uri + '/';
    for (size_t i = 0; i < uri.size(); i++)
    {
        if (uri[i] == '\\') // a verifier
            throw std::runtime_error("error in configuration file's syntax (location)");
    }
}

std::string &LocationConfig::getUri()
{
	return (uri);
}

std::vector<Directive> &LocationConfig::getDir()
{
	return (dir);
}

void LocationConfig::print_location()
{
    std::cout << "LOCATION" << std::endl;
    std::cout << "location uri =" << uri << std::endl;
    for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
    {
        Directive &dir = *it;
        dir.print_directive();
    }
}


LocationConfig::LocationConfig(const LocationConfig &src) : dir(src.dir), uri(src.uri), conf(src.conf)
{
}

LocationConfig::~LocationConfig()
{

}

LocationConfig &LocationConfig::operator=(const LocationConfig &src)
{
    if (this != &src)
    {
        dir = src.dir;
        uri = src.uri;
		conf = src.conf;
    }
    return (*this);
}

LocationConfig::LocationConfig() : dir(), uri(), conf(NULL)
{
}
