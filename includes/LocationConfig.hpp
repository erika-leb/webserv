#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include "all.hpp"
#include "Directive.hpp"
#include "ServerConfig.hpp"

class Directive;
class ServerConfig;

class LocationConfig {

	public:
        LocationConfig(std::fstream &temp, std::string &line, ServerConfig *sconf);
        LocationConfig(const LocationConfig &src);
        ~LocationConfig();
        LocationConfig &operator=(const LocationConfig &src);

        void get_uri(std::string &line);
		void addOtherDir();

        void print_location();

	private:
		std::vector<Directive> dir;
        std::string uri;

		ServerConfig *conf;

		LocationConfig();
} ;

#endif
