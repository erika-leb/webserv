#include "all.hpp"
#include "server.hpp"
#include "Request.hpp"

int main()
{
	try
	{
		Server serv;
		signal(SIGINT, Server::handleSigint);
		serv.launch();
		// initServ();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}
