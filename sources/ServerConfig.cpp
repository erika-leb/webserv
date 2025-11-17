#include "ServerConfig.hpp"

ServerConfig::ServerConfig(std::fstream &temp) : dir(), locs()
{
    std::string line;
    int loc = 0;

    while (1)
	{
        // std::cout << "linguine = " << line << std::endl;
        // perror("domaine");
        if (!((line.size() >= 8 && line.substr(0, 8) == "location") || line == "server"))
            std::getline(temp, line);
        // perror("préjugé");
        trim_line(line);
        // std::cout << "line = " << line << std::endl;
        if (!temp.eof() && temp.fail())
		    throw std::runtime_error("error while reading temp file" + static_cast<std::string>(strerror(errno)));
        // perror("catlina");
        // std::cout << "line = " << line << std::endl;
        // else if (temp.eof() || line == "}")
        else if (temp.eof() || line == "server")
        {
            // perror("puede");
            // std::cout << "line = " << line << std::endl;
            break ;
        }    
        else if (loc == 0 && line.substr(0, 8) != "location")
        {
            // perror("corazon"); 
            dir.push_back(Directive(line));
        }
        // perror("loca");
        else if (line.substr(0, 8) == "location")
        {
            loc++;
            // get_uri(line);
            // line = line.substr(8);
            // trim_line(line);
            //ici on recupere l'url !!!!!!
            locs.push_back(LocationConfig(temp, line));
            //on traite le reste
        }
        // ICI IL FAUT VERIFIER QUE LES  2 ARGS OBLIG SONT LA
        // else
        // {
        //     // std::cout << "luine = " << line << std::endl;
        //     throw std::runtime_error("error configuration file should contain at least one server");
        // }
    }
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

ServerConfig::ServerConfig(const ServerConfig &src) : dir(src.dir), locs(src.locs)
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
    }
    return (*this);
}

ServerConfig::ServerConfig() : dir(), locs()
{
}