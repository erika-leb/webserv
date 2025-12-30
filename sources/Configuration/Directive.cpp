#include "Directive.hpp"

Directive::Directive(std::string line) : name(""), nbArg(0), arg(), sizeMax(-1) // verifier que la directive est correcte sinon trhow
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
	if (arg[0].empty())
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0].find("..") != std::string::npos)
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0].find("~") != std::string::npos)
		throw std::runtime_error("error configuration file's root directive");
	if (arg[0][arg[0].size() - 1] == '/')
		arg[0] = arg[0].substr(0, arg[0].size() - 1);
	// if (arg[0][0] == '/')
	// 	arg[0] = arg[0].substr(1, arg[0].size());
	if (arg[0] == "sources")
		throw std::runtime_error("error configuration file's root directive : sources files cannot be modified");
	if (arg[0] == "default_documents")
		throw std::runtime_error("error configuration file's root directive : default_documents files cannot be modified");
	if (arg[0] == "includes")
		throw std::runtime_error("error configuration file's root directive : includes files cannot be modified");
	if (arg[0] == "")
		throw std::runtime_error("error configuration file's root directive : root should not be empty");
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

	if (arg[1][0] != '/')
		arg[1] = '/' + arg[1];
	if (arg[1][arg[1].size() - 1] == '/')
		arg[1] = arg[1].substr(0, arg[1].size() - 1);
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

void Directive::checkAutoindex()
{
	if (nbArg != 1)
		throw std::runtime_error("error in configuration file : autoindex directive should have one attribute");
	if (arg[0] != "on" && arg[0] != "off")
		throw std::runtime_error("error in configuration file : autoindex directive should have 'on' or 'off' as an attribute");
}

void Directive::checkIndex()
{
	if (nbArg != 1)
		throw std::runtime_error("error in configuration file : index directive should have one attribute");
	if (arg[0][0] != '/')
		arg[0] = '/' + arg[0];
}

void Directive::checkMaxBody()
{
	unsigned long long nb = 0;
	size_t i = 0;
	int digit;
	char u;
	unsigned long long multiplier = 1;

	if (nbArg != 1 || arg[0] == "")
		throw std::runtime_error("error in configuration file : Max Size directive should have one attribute");

	while (i < arg[0].size() && std::isdigit(arg[0][i]))
	{
		digit = arg[0][i] - '0';
		if (nb > ((std::numeric_limits<unsigned long long>::max() - digit) / 10))
			throw std::runtime_error("error in configuration file : size overflow in Max Size directive");
		nb = nb * 10 + digit;
		i++;
	}

	if (i == 0)
		throw std::runtime_error("error in configuration file : No numeric part in size");

	if (i < arg[0].size())
	{
		u = arg[0][i];
		if (i + 1 != arg[0].size())
			throw std::runtime_error("error in configuration file : Invalid unit format in max size directive");
		if (u == 'k' || u == 'K')
			multiplier = 1024ULL;
		else if (u == 'm' || u == 'M')
			multiplier = 1024ULL * 1024ULL;
		else if (u == 'g' || u == 'G')
			multiplier = 1024ULL * 1024ULL * 1024ULL;
		else
			throw std::runtime_error("error in configuration file : Unknown size unit in max size directive");
	}
	if (nb > std::numeric_limits<unsigned long long>::max() / multiplier)
		throw std::runtime_error("error in configuration file : size too large in max size directive");
	sizeMax = nb * multiplier;
}

void Directive::checkCgi()
{
	char c;

	if (nbArg != 2)
		throw std::runtime_error("error in configuration file : cgi directive should have 2 attributes");
	if (arg[0].size() < 2 || arg[0][0] != '.')
		throw std::runtime_error("error in configuration file : Invalid cgi extension");
	for (size_t i = 0; i < arg[0].size(); i++)
	{
		c = arg[0][i];
		if (c == ';' || c == '/' || c == '\0' || c == '\n' || c == '\r' || c == ' ' || c == '\t')
			throw std::runtime_error("error in configuration file : Invalid character in CGI extension");
	}
	if (arg[1].empty())
		throw std::runtime_error("error in configuration file : Invalid cgi path");
	for (size_t i = 0; i < arg[1].size(); i++)
	{
		c = arg[1][i];
		if (c == ';' || c == '\0' || c == '\n' || c == '\r')
			throw std::runtime_error("error in configuration file : Invalid character in CGI path");
	}
}

// void Directive::checkUpload()
// {
// 	if (nbArg != 1 || arg[0] == "")
// 		throw std::runtime_error("error in configuration file : upload directive should have one attribute");
// 	if (arg[0][0] != '/')
// 		arg[0] = '/' + arg[0];
// }

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
	else if (name == "index")
		checkIndex();
	else if (name == "autoindex")
		checkAutoindex();
	else if (name == "client_max_body_size")
		checkMaxBody();
	else if (name == "cgi_handler")
		checkCgi();
	// else if (name == "upload_path")
	// 	checkUpload();
	// else if (name == "listen")
	// {}
	else if (name != "listen")
		throw std::runtime_error("error in configuration file's directive : bad directive"); // a rajouter a la fin pour rendre plus contraignant le fichier de conf  ou pas ?
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

unsigned long long Directive::getSizeMax()
{
	return sizeMax;
}

Directive::Directive(const Directive &src) : name(src.name), nbArg(src.nbArg), arg(src.arg), sizeMax(src.sizeMax )
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
		sizeMax = src.sizeMax;
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

Directive::Directive() : name(""), nbArg(0), arg(), sizeMax(-1)
{
}
