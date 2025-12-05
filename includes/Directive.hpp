#ifndef DIRECTIVE_HPP
# define DIRECTIVE_HPP

#include "all.hpp"

class Directive {

    public:
            Directive(std::string line);
			Directive();
            Directive(const Directive &src);
            ~Directive();
            Directive &operator=(const Directive &src);

			std::string &getName();
			int getNbArg() const;
			std::vector<std::string> &getArg();

			void checkBasicDir();
			void checkRoot();
			void checkError();
			void checkRedir();
			void checkAutoindex();
			void checkIndex();

            void print_directive();

    private:
            std::string name;
            int nbArg;
            std::vector<std::string> arg;

            // Directive();

} ;

#endif
