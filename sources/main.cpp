#include "all.hpp"
#include "Server.hpp"
#include "Request.hpp"


void parseConfig(std::string config_path)
{
	std::ifstream file(config_path.c_str());
	if (!file)
		throw std::runtime_error("could not open configuration file" + static_cast<std::string>(strerror(errno)));


}

int main(int ac, char **av)
{
	std::string config_path;
	if (ac > 1)
		config_path = av[1];
	else
		config_path = "default_conf.conf";
	try
	{
		parseConfig(config_path);
		Server serv;
		signal(SIGINT, Server::handleSigint);
		serv.launch();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}
