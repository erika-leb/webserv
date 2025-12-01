#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include "all.hpp"
#include "Directive.hpp"
#include "LocationConfig.hpp"
#include "GlobalConfig.hpp"

class Directive;
class LocationConfig;
class GlobalConfig;

class ServerConfig {

	public:
        ServerConfig(std::fstream &temp, GlobalConfig *gconf);
        ServerConfig(const ServerConfig &src);
        ~ServerConfig();
        ServerConfig &operator=(const ServerConfig &src);

		void checkListen();
		bool isValidListArg(std::string &s);
		bool isValidPort(std::string &s);
		bool isValidHostPort(std::string &s);
		bool isValidIPv4(std::string &s);

		void addGlobalDir();

		std::vector<Directive>&		getDir();
		int							getPort();
		std::string					&getIp();
		GlobalConfig				*getGlobConf();
		std::vector<LocationConfig> &getLocation();

		void checkBasicDir();
		void checkAllowedDirective();

		void print_server();

	private:
		std::vector<Directive> dir;
		std::vector<LocationConfig> locs;

		GlobalConfig *conf;

		std::string _ip;
		int _port;

		ServerConfig();

} ;

#endif
