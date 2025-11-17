#include "LocationConfig.hpp"

LocationConfig::LocationConfig(std::fstream &temp, std::string &line) : dir(), uri() // recuperer ce qu'il y a apres location
{
    // int loc = 0;
    // perror("lola");
    get_uri(line);
    // std::cout << "uri" << uri << std::endl;

    while (1)
	{
        std::getline(temp, line);
        trim_line(line);
        if (!temp.eof() && temp.fail())
		    throw std::runtime_error("error while reading temp file" + static_cast<std::string>(strerror(errno)));
        // perror("catlina");
        // std::cout << "LINE = " << line << std::endl;
        // std::cout << "LINE substr = " << line.substr(8) << std::endl;
        if (temp.eof() || (line.size() >= 8 && line.substr(0, 8) == "location") || (line.size() >= 6 && line == "server"))
        {
            // perror("malleur");
            break ;
        }
        dir.push_back(Directive(line));
	}

}

// void LocationConfig::get_uri(std::string &line)
// {
//     size_t i = 0;
//     int nbarg = 0;
//     std::string word;
 
//     line = line.substr(8);
//     trim_line(line);

//     while (i < line.size() && !std::isspace(line[i]))
//         i++;
//     uri = line.substr(0, i);
//     line = line.substr(i);
//     trim_line(line);
//     for (size_t j = 0; j < line.size(); j++)
//     {
//         if (std::isspace(line[j]))
//         {
//             if (!word.empty())
//             {
//                 nbarg++;
//                 word.clear();
//             }
//         }
//         else
//             word = word + line[j];
//     }
//     if (!word.empty())
//         nbarg++;
//     if (nbarg != 0)
// 		throw std::runtime_error("error in configuration file's syntax (location)");
// }

void LocationConfig::get_uri(std::string &line)
{
    size_t i = 0;
    // int nbarg = 0;
 
    line = line.substr(8);
    trim_line(line);

    while (i < line.size() && !std::isspace(line[i]))
        i++;
    uri = line.substr(0, i);
    line = line.substr(i);
    trim_line(line);
    for (size_t j = 0; j < line.size(); j++)
    {
        if (std::isspace(line[j]))
		    throw std::runtime_error("error in configuration file's syntax (location)");
    }
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


LocationConfig::LocationConfig(const LocationConfig &src) : dir(src.dir), uri(src.uri)
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
    }
    return (*this);
}

LocationConfig::LocationConfig() : dir(), uri()
{
}