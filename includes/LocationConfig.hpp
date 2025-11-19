#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include "all.hpp"
#include "Directive.hpp"

class Directive;

class LocationConfig {

	public:
        LocationConfig(std::fstream &temp, std::string &line);
        LocationConfig(const LocationConfig &src);
        ~LocationConfig();
        LocationConfig &operator=(const LocationConfig &src);

        void get_uri(std::string &line);
        void print_location();

	private:
		std::vector<Directive> dir;
        std::string uri;

		LocationConfig();
} ;

#endif