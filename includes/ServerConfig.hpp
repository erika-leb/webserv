#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "all.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"

class Directive;
class LocationConfig;

class ServerConfig {

	public:
        ServerConfig(std::fstream &temp);
        ServerConfig(const ServerConfig &src);
        ~ServerConfig();
        ServerConfig &operator=(const ServerConfig &src);

		void print_server();

	private:
		std::vector<Directive> dir;
		std::vector<LocationConfig> locs;

		ServerConfig();
} ;

#endif