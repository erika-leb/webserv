#ifndef DIRECTIVE_HPP
# define DIRECTIVE_HPP

#include "all.hpp"

class Directive {

    public:
            Directive(std::string &line);
            Directive(const Directive &src);
            ~Directive();
            Directive &operator=(const Directive &src);

            void print_directive();

    private:
            std::string name;
            int nbArg;
            std::vector<std::string> arg;

            Directive();
		
} ;

#endif