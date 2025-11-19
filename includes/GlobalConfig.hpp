#ifndef GLOBALCONFIG_HPP
# define GLOBALCONFIG_HPP

#include "all.hpp"
#include "Directive.hpp"
#include "ServerConfig.hpp"

class Directive;
class ServerConfig;

class GlobalConfig {

	public:
        GlobalConfig(std::fstream &temp);
        GlobalConfig(const GlobalConfig &src);
        ~GlobalConfig();
        GlobalConfig &operator=(const GlobalConfig &src);

		std::vector<Directive>& getDir();

		void print_config();

	private:
		std::vector<Directive> dir;
		std::vector<ServerConfig> servs;

		GlobalConfig();
} ;

#endif
