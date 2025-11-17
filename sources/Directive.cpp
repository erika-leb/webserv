#include "Directive.hpp"

Directive::Directive(std::string &line) : name(""), nbArg(0), arg() // verifier que la directive est correcte sinon trhow
{
    size_t i = 0;
    std::string word;

    // perror("solo");
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
        arg = arg;
    }
    return (*this);
}

void Directive::print_directive()
{
    std::cout << "Directive name: " << name << std::endl;
    std::cout << "Arguments:" << std::endl;
    for (size_t i = 0; i < arg.size(); ++i)
    {
        std::cout << "  [" << i << "] " << arg[i] << std::endl;
    }
    std::cout << "nb of argument" << nbArg << std::endl;
}


Directive::Directive() : name(""), nbArg(0), arg()
{
}