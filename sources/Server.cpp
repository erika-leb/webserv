#include "Server.hpp"

volatile sig_atomic_t Server::flag = 0;

void Server::cleanClose()
{
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	// for (std::map<int, ListenInfo>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
		// close (_fdListen[i]);
		close(it->first);
	close(_poll);
}

unsigned long Server::getIPAddr(std::string &ip, struct addrinfo **res)
{
	struct addrinfo hints;
	int status;

	ft_memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(ip.c_str(), NULL, &hints, res);
	if (status != 0 || res == NULL)
	{
		cleanClose();
		freeaddrinfo(*res);
        throw std::runtime_error("getaddrinfo() failed: " + std::string(gai_strerror(status)));
	}
	return (((struct sockaddr_in *)(*res)->ai_addr)->sin_addr.s_addr);
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

Server::Server(GlobalConfig *config) : config(config)
{
	std::vector<ServerConfig> servs = this->config->getServ();
	int fd;
	// ListenInfo info;

	_poll = epoll_create(1);
	if (_poll == -1)
		throw std::runtime_error("epoll_create failed"+ static_cast<std::string>(strerror(errno)));
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
		// initFdListen(_fdListen[i], servs[i].getPort(), servs[i].getIp());
	}
}

void Server::modifyEvent(int fd, uint32_t events)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = events;
	epoll_ctl(_poll, EPOLL_CTL_MOD, fd, &event);
}

void Server::deleteSocket(int client_fd)
{
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (client_fd == (*it)->getFd())
		{
			epoll_ctl(_poll, EPOLL_CTL_DEL, client_fd, NULL);
			delete (*it); // pointer freed
			_clients.erase(it); // pointer remved from the list
			break ;
		}
	}
}

bool Server::is_listen_fd(int fd)
{
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	// for (std::map<int, ListenInfo>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
		if (it->first == fd)
			return (true);
    return false;
}

void Server::NewIncomingConnection(int fd, struct sockaddr_in cli, struct epoll_event &event)
{
	socklen_t cli_len;
	int client_fd;

	cli_len = sizeof(cli);
	client_fd = accept(fd, (struct sockaddr *)&cli, &cli_len);
	if (client_fd < 0)
	{
		std::cout << "accept() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
		return ;
	}
	if (make_non_blocking(client_fd) == -1)
	{
		close(client_fd);
		std::cerr << "failed to set client socket non-blocking" << std::endl;
		return ;
	}
	event.data.fd = client_fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(_poll, EPOLL_CTL_ADD, client_fd, &event);
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	// for (std::map<int, ListenInfo>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	{
		if (it->first == fd)
			_clients.push_back(new Client(client_fd, it->second));
	}
	// _clients.push_back(new Client(client_fd));
	std::cout << date(LOG) << ": Client(" << client_fd << ") connected" << std::endl;
}

void Server::prepareResponse(char buff[MAXLINE], std::string& tmp, int client_fd, Client *cli)
{
	std::cout << date(LOG) << ": Request from client(" << client_fd << ")" << std::endl;
	cli->addBuff(buff);
	DEBUG_MSG("\nReceived: {\n" << cli->getBuff() << "}");
	if ((cli->getBuff()).find("\r\n\r\n") != std::string::npos) // Voir plus tard si on essaye de traiter la requete au fur et a mesure
	{
		Request req(*cli);
		req.parseHttp();
		std::string cgiFolder(".py"); // erase this line and replace the argument of the function with the actual folder from configuration file
		if (req.is_cgi(cgiFolder) && (req.getsCode() == 200)) { // check if we are in the cgi folder and that there are no problem
			cli->setCgi(new Cgi(req.getPathFile(), req.getAction(), *cli, req.getServIp(), req.getServPort()));
			cli->getCgi()->handleCGI_fork(_poll);
		}
		else {
			req.handleAction(req.getAction());
			tmp = req.makeResponse();
			modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
		}
		cli->clearRequestBuff(); // erase the processed request
	}
}

int Server::reveiveRequest(int i, std::string& tmp)
{
	int client_fd;
	char buff[MAXLINE];
	int n;

	client_fd = _events[i].data.fd;
	n = read(client_fd, buff, sizeof(buff) - 1);
	if (n < 0)
	{
		deleteSocket(client_fd);
		std::cerr << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
		return (1);
	}
	else if (n == 0)
	{
		std::cout << date(LOG) << ": Client(" << client_fd << ") disconnected (read() == 0)" << std::endl;
		deleteSocket(client_fd);
		return (1);
	}
	else
	{
		buff[n] = '\0';
		for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			if (client_fd == (*it)->getFd())
			{
				(*it)->setlastConn(std::time(NULL));
				prepareResponse(buff, tmp, client_fd, (*it));
				return (1);
			}
		}
	}
	return (0);
}

int Server::sendRequest(int i, std::string tmp)
{
	int client_fd;
	int n;

	client_fd = _events[i].data.fd;
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (client_fd == (*it)->getFd())
		{
			n = send(client_fd, (*it)->getSendBuffer(), (*it)->setSendSize(), 0);
			std::cout << date(LOG) << ": Send " << n << " B to client(" << client_fd << ") [" << tmp << "]" << std::endl;
			if (n > 0)
			{
				(*it)->sendBuffErase(n);
				if (((*it)->getToSend()).empty())
				{
					modifyEvent(client_fd, EPOLLIN);
				}
			}
			else if (n == 0)
				return (1) ;
			else
			{
				deleteSocket(client_fd);
				std::cerr << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
				return (1) ;
			}
			if ((*it)->isCon() == false)
			{
				std::cout << date(LOG) << ": Server closed connection to client(" << client_fd << ") [" << (*it)->isCon() << "]" << std::endl;
				deleteSocket(client_fd);
			}
			return (1) ;
		}
	}
	return (0);
}

void Server::checkTimeOut()
{
	time_t now = std::time(NULL);

	for (std::vector<Client *>::size_type i = 0; i < _clients.size(); i++)
	{
		if (now - _clients[i]->getlastConn() > TIMEOUT_SECONDS)
		{
			std::cout << date(LOG) << ": Client(" << _clients[i]->getFd() << ") disconnected (TIMEOUT)" << std::endl;
			deleteSocket(_clients[i]->getFd());
		}
	}
}

int Server::timeOut()
{
	time_t now = std::time(NULL);
	int res = TIMEOUT_SECONDS;
	int elapsed;

	for (std::vector<Client *>::size_type i = 0; i < _clients.size(); i++)
	{
		elapsed = now - _clients[i]->getlastConn();
		if (TIMEOUT_SECONDS - elapsed > 0 && TIMEOUT_SECONDS - elapsed < res)
			res = TIMEOUT_SECONDS - elapsed;
	}
	return ((res + 2) * 1000);
}

void Server::launch()
{
	struct epoll_event event;
	struct sockaddr_in cli;
	int d;
	std::string tmp; //revoir avec Thibault si a modifier

	while(flag == 0)
	{
		d = epoll_wait(_poll, _events, SOMAXCONN, timeOut());
		// d = epoll_wait(_poll, _events, SOMAXCONN, -1);
		for (int i = 0; i < d; i++)
		{
			if (_events[i].events & EPOLLIN)
			{
				if (is_listen_fd(_events[i].data.fd) == true)
					NewIncomingConnection(_events[i].data.fd, cli, event);
				else
				{
					if (is_pipe_fd(_events[i].data.fd) == true) {
						receiveCgi(i, tmp);
						break;
					}
					else {
						if (reveiveRequest(i, tmp) == 1)
							break;
					}
				}
			}
			if (_events[i].events & EPOLLOUT)
			{

				if (sendRequest(i, tmp) == 1)
				{
					break;
				}
			}
		}
		checkTimeOut();
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

void Server::handleSigint(int sig)
{
	if (sig == SIGINT)
	{
		std::cout << date(LOG) << ": Server shutdown" << std::endl;
		if (std::remove("temp.txt") != 0)
		    std::cerr << "Erreur lors de la suppression du fichier." << std::endl;
		flag = 1;
		// exit(0);
	}
}

bool Server::is_pipe_fd( int fd ) {
	for (std::vector< Client *>::iterator it=_clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getCgi() != NULL) {
			if ((*it)->getCgi()->getFd(READ) == fd)
				return true;
		}
	}
	return false;
}

void Server::receiveCgi( int i, std::string tmp ) {
	int pipe_fd;

	(void)tmp;
	pipe_fd = _events[i].data.fd;
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getCgi() == NULL)
			continue;
		if (pipe_fd == (*it)->getCgi()->getFd(READ)) {
			(*it)->getCgi()->handleCGI_pipe(pipe_fd);
			epoll_ctl(_poll, EPOLL_CTL_DEL, pipe_fd, NULL);
			// (*it)->deleteCgi(); // is this function really needed ?
			modifyEvent((*it)->getFd(), EPOLLIN | EPOLLOUT);
		}
	}
	return ;
}
