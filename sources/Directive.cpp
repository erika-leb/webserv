#include "Directive.hpp"

Directive::Directive(std::string line) : name(""), nbArg(0), arg() // verifier que la directive est correcte sinon trhow
{
    size_t i = 0;
    std::string word;

    while (i < line.size() && !std::isspace(line[i]))
        i++;
    name = line.substr(0, i);
    line = line.substr(i);
    trim_line(line);
    for (size_t j = 0; j < line.size(); j++)
    {
        if (std::isspace(line[j]))
        {
            if (!word.empty())
            {
                arg.push_back(word);
                nbArg++;
                word.clear();
            }
        }
        else
            word = word + line[j];
    }
    if (!word.empty())
    {
        nbArg++;
        arg.push_back(word);
        word.clear();
    }
	checkBasicDir();
}

void Directive::checkRoot()
{
	// std::cout << "deb - " << arg[0] <<std::endl;
	// std::cout << "arg[0][arg[0].size()] - " << arg[0][arg[0].size() - 1] <<std::endl;
	if (arg[0].empty())
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0].find("..") != std::string::npos)
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0].find("~") != std::string::npos)
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0][arg[0].size() - 1] == '/')
	{
		// perror("I");
		arg[0] = arg[0].substr(0, arg[0].size() - 1);
		// std::cout << "arg - " << arg[0] <<std::endl;
	}
	if (arg[0][0] == '/')
		arg[0] = arg[0].substr(1, arg[0].size());
	// std::cout << "fin - " << arg[0] <<std::endl;
	// perror("It");
}

void Directive::checkError()
{
	int code;

	if (nbArg != 2)
		throw std::runtime_error("error in configuration file : error_page directive");
	if (arg[0].size() != 3)
		throw std::runtime_error("error in configuration file : error_page directive");
	for (size_t i = 0; i < arg[0].size(); i++)
	{
		if (arg[0][i] < '0' || arg[0][i] > '9')
			throw std::runtime_error("error in configuration file : error_page directive");
	}
	code = std::atoi(arg[0].c_str());
	if ( code < 300 || code > 599)
		throw std::runtime_error("error in configuration file : error_page directive");
	if (arg[1].empty())
		throw std::runtime_error("error in configuration file : error_page directive"); //usefull ?

	// std::cout << "arg av = " << arg[1] << std::endl;
	if (arg[1][0] != '/')
		arg[1] = '/' + arg[1];
	if (arg[1][arg[1].size() - 1] == '/')
		arg[1] = arg[1].substr(0, arg[1].size() - 2);
	// std::cout << "arg ap = " << arg[1] << std::endl;
}

void Directive::checkRedir()
{
	int code;

	if (nbArg != 2)
		throw std::runtime_error("error in configuration file : redirection directive");
	if (arg[0].size() != 3)
		throw std::runtime_error("error in configuration file : redirection directive");
	for (size_t i = 0; i < arg[0].size(); i++)
	{
		if (arg[0][i] < '0' || arg[0][i] > '9')
			throw std::runtime_error("error in configuration file : redirection directive");
	}
	code = std::atoi(arg[0].c_str());
	if ( code != 301 && code != 302 && code != 307 && code != 308)
		throw std::runtime_error("error in configuration file : redirection code should only be 301, 302, 307 or 308");
	if (arg[1].empty())
		throw std::runtime_error("error in configuration file : redirection directive"); //usefull ?
}

void Directive::checkBasicDir()
{
	if (nbArg == 0)
		throw std::runtime_error("error in configuration file : directive should have at least one attribute"); //usefull ?
	if (name == "root")
		checkRoot();
	else if (name == "error_page")
		checkError();
	else if (name == "allow_methods")
	{
		for (std::string::size_type i = 0; i < arg.size(); i++)
		{
			if (arg[i] != "GET" && arg[i] != "DELETE" && arg[i] != "POST")
				throw std::runtime_error("error configuration file : allowed methods can only be GET, POST AND DELETE");
		}
	}
	else if (name == "return")
		checkRedir();

	// else
	// 	throw std::runtime_error("error in configuration file's directive"); // a rajouter a la fin pour rendre plus contraignant le fichier de conf  ou pas ?
}

std::string &Directive::getName()
{
	return (name);
}

int Directive::getNbArg() const
{
	return (nbArg);
}

std::vector<std::string> &Directive::getArg()
{
	return (arg);
}

Directive::Directive(const Directive &src) : name(src.name), nbArg(src.nbArg), arg(src.arg)
{
}

Directive::~Directive()
{

}

Directive &Directive::operator=(const Directive &src)
{
    if (this != &src)
    {
        name = src.name;
        nbArg = src.nbArg;
        arg = src.arg;
    }
    return (*this);
}

// void Directive::print_directive()
// {
//     std::cout << "Directive name: " << name << std::endl;
//     std::cout << "Arguments:" << std::endl;
//     for (size_t i = 0; i < arg.size(); ++i)
//     {
//         std::cout << "  [" << i << "] " << arg[i] << std::endl;
//     }
//     std::cout << "nb of argument" << nbArg << std::endl;
// }

void Directive::print_directive()
{
    std::cout << "Directive: " << name;
    for (size_t i = 0; i < arg.size(); ++i)
    {
        std::cout <<" " <<  arg[i] << "  [" << i << "] " ;
    }
	std::cout << std::endl;
}

Directive::Directive() : name(""), nbArg(0), arg()
{
}
