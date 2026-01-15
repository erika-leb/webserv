#include "Server.hpp"
#include "all.hpp"

volatile sig_atomic_t Server::flag = 0;

Server::Server(GlobalConfig *config) : config(config)
{
	std::vector<ServerConfig> servs = this->config->getServ();
	int fd;
	// ListenInfo info;

	_poll = epoll_create(1);
	if (_poll == -1)
		throw std::runtime_error("epoll_create failed"+ static_cast<std::string>(strerror(errno)));
	if ( fcntl(_poll, F_SETFD, FD_CLOEXEC) < 0) {
		std::cerr << "fcntl() failed" + static_cast<std::string>(strerror(errno)) << std::endl;
	}
	for (std::vector<ServerConfig>::size_type i = 0; i < servs.size(); i++)
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
			throw std::runtime_error("socket failed"+ static_cast<std::string>(strerror(errno)));
		// info.fd = fd;
		// info.ip = servs[i].getIp();
		// info.port = servs[i].getPort();
		// _fdListen[fd] = servs[i];
		_fdListen.insert(std::make_pair(fd, servs[i]));

		// _fdListen[fd] = info;
		// _fdListen.push_back(socket(AF_INET, SOCK_STREAM, 0));
		initFdListen(fd, servs[i].getPort(), servs[i].getIp());
		// std::cout << "fd d'ecoute = " << fd << std::endl;
		// initFdListen(_fdListen[i], servs[i].getPort(), servs[i].getIp());
	}
}

Server::~Server()
{
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	// for (std::map<int, ListenInfo>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
		close(it->first);
	for (std::vector< Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close((*it)->getFd());
		delete (*it);
	}
	_clients.clear();
	close(_poll);
}

Server::Server(const Server &src)
{
	(void) src;
}

Server &Server::operator=(const Server &rhs)
{
	(void)rhs;
	return (*this);
}

void Server::initFdListen(int fd, int port, std::string &ip)
{
	struct sockaddr_in addr;
	struct epoll_event event;
	struct addrinfo *res = NULL;

	if (fd == -1)
		throw std::runtime_error("socket() failed " + static_cast<std::string>(strerror(errno)));

	int optval = 1; // a enlever apres
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); //a enlever apres

	ft_memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = getIPAddr(ip, &res);
	addr.sin_port = htons(port);

	freeaddrinfo(res);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) // = to write servor's number down in the phonebook, connect a port to a fd
	{
		cleanClose();
		throw std::runtime_error("bind() failed " + static_cast<std::string>(strerror(errno)));
	}

	if(listen(fd, SOMAXCONN) < 0)
	{
		cleanClose();
		throw std::runtime_error("listen() failed " + static_cast<std::string>(strerror(errno)));
	}

	if (make_non_blocking(fd) == -1)
	{
		cleanClose();
		throw std::runtime_error("failed to set socket non-blocking " + static_cast<std::string>(strerror(errno)));
	}

	event.data.fd = fd;
	event.events = EPOLLIN;
	epoll_ctl(_poll, EPOLL_CTL_ADD, fd, &event);
}
