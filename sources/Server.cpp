#include "Server.hpp"

volatile sig_atomic_t Server::flag = 0;

void Server::initFdListen(int fd, int port)
{
	struct sockaddr_in addr;
	struct epoll_event event;

	if (fd == -1)
		throw std::runtime_error("socket() failed " + static_cast<std::string>(strerror(errno)));

	int optval = 1; // a enlever apres
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); //a enlever apres

	ft_memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr))) // = to write servor's number down in the phonebook
	{
		close(fd);
		throw std::runtime_error("bind() failed " + static_cast<std::string>(strerror(errno)));
	}

	if(listen(fd, SOMAXCONN) < 0)
	{
		close(fd);
		throw std::runtime_error("listen() failed " + static_cast<std::string>(strerror(errno)));
	}

	if (make_non_blocking(fd) == -1)
		throw std::runtime_error("failed to set socket non-blocking " + static_cast<std::string>(strerror(errno)));

	event.data.fd = fd;
	event.events = EPOLLIN;
	epoll_ctl(_poll, EPOLL_CTL_ADD, fd, &event);
}

Server::Server(GlobalConfig *config) : config(config)
{
	std::vector<ServerConfig> servs = this->config->getServ();

	_poll = epoll_create(1);
	if (_poll == -1)
		throw std::runtime_error("epoll_create failed"+ static_cast<std::string>(strerror(errno)));
	for (std::vector<ServerConfig>::size_type i = 0; i < servs.size(); i++)
	{
		_fdListen.push_back(socket(AF_INET, SOCK_STREAM, 0));
		initFdListen(_fdListen[i], servs[i].getPort());
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
    for (size_t i = 0; i < _fdListen.size(); i++)
        if (_fdListen[i] == fd)
            return true;
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
	_clients.push_back(new Client(client_fd));
	std::cout << date(LOG) << ": Client(" << client_fd << ") connected" << std::endl;
}

int Server::reveiveRequest(int i)
{
	int client_fd;
	char buff[MAXLINE];
	int n;

	std::string tmp;

	client_fd = _events[i].data.fd;
	n = read(client_fd, buff, sizeof(buff) - 1);
	// printf("n = %d i = %d, fd = %d\n", n, i, client_fd);
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
				std::cout << date(LOG) << ": Request from client(" << client_fd << ")" << std::endl;
				(*it)->addBuff(buff);
				DEBUG_MSG("\nReceived: {\n" << (*it)->getBuff() << "}");
				if (((*it)->getBuff()).find("\r\n\r\n") != std::string::npos) // Voir plus tard si on essaye de traiter la requete au fur et a mesure
				{
					Request req(*(*it));
					req.parseHttp();
					std::string cgiFolder("/cgi"); // erase this line and replace the argument of the function with the actual folder from configuration file
					// if (req.is_cgi(cgiFolder)) // check if we are in the cgi folder
					// 	handleCGI(req.getPathFile());
					req.handleAction(req.getAction());
					tmp = req.makeResponse(); // close or keep-alive depending on the value of connection
					modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
					(*it)->clearRequestBuff(); // erase the processed request
				}
				return (1);
			}
		}
	}
	return (0);
}

void Server::launch()
{
	struct epoll_event event;
	struct sockaddr_in cli;
	int client_fd;
	// char buff[MAXLINE];
	int n;
	int d;

	std::string tmp;
	int i = 0;

	while(flag == 0) // faut penser a arreter ailleurs aussi
	{
		d = epoll_wait(_poll, _events, SOMAXCONN, -1);
		i++;
		for (int i = 0; i < d; i++)
		{
			// printf("d = %d\n", d);
			// printf("event = %d\n", _events[i].events);
			// printf("EPOLLIN = %d\n", EPOLLIN);
			// printf("EPOLLOUT = %d\n", EPOLLOUT);
			if (_events[i].events & EPOLLIN)
			{
				if (is_listen_fd(_events[i].data.fd) == true)
					NewIncomingConnection(_events[i].data.fd, cli, event);
				else
				{
					if (reveiveRequest(i) == 1)
						break ;
				}
				// {
				// 	client_fd = _events[i].data.fd;
				// 	n = read(client_fd, buff, sizeof(buff) - 1);
				// 	// printf("n = %d i = %d, fd = %d\n", n, i, client_fd);
				// 	if (n < 0)
				// 	{
				// 		deleteSocket(client_fd);
				// 		std::cerr << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
				// 		break ;
				// 	}
				// 	else if (n == 0)
				// 	{
				// 		std::cout << date(LOG) << ": Client(" << client_fd << ") disconnected (read() == 0)" << std::endl;
				// 		deleteSocket(client_fd);
				// 		break ;
				// 	}
				// 	else
				// 	{
				// 		buff[n] = '\0';
				// 		for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
				// 		{
				// 			if (client_fd == (*it)->getFd())
				// 			{
				// 				std::cout << date(LOG) << ": Request from client(" << client_fd << ")" << std::endl;
				// 				(*it)->addBuff(buff);
				// 				DEBUG_MSG("\nReceived: {\n" << (*it)->getBuff() << "}");
				// 				if (((*it)->getBuff()).find("\r\n\r\n") != std::string::npos) // Voir plus tard si on essaye de traiter la requete au fur et a mesure
				// 				{
				// 					Request req(*(*it));
				// 					req.parseHttp();

				// 					std::string cgiFolder("/cgi"); // erase this line and replace the argument of the function with the actual folder from configuration file
				// 					// if (req.is_cgi(cgiFolder)) // check if we are in the cgi folder
				// 					// 	handleCGI(req.getPathFile());
				// 					req.handleAction(req.getAction());
				// 					tmp = req.makeResponse(); // close or keep-alive depending on the value of connection
				// 					modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
				// 					(*it)->clearRequestBuff(); // erase the processed request
				// 				}
				// 				break ;
				// 			}
				// 		}
				// 	}
				// }
			}
			if (_events[i].events & EPOLLOUT)
			{
				client_fd = _events[i].data.fd;
				// printf("fd to send = %d\n", client_fd);
				for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
				{
					// printf("fd de liste = %d\n", (*it)->getFd());
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
						{
							// deleteSocket(client_fd);
							break ;
						}
						else
						{
							deleteSocket(client_fd);
							std::cerr << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
							break ;
						}
						if ((*it)->isCon() == false)
						{
							std::cout << date(LOG) << ": Server closed connection to client(" << client_fd << ") [" << (*it)->isCon() << "]" << std::endl;
							deleteSocket(client_fd);
						}
						break ;
					}
				}
			}
		}
	}
}

Server::Server(const Server &src)
{
	(void) src;
}

Server::~Server()
{
	for (std::vector<int>::size_type i = 0; i < _fdListen.size(); i++)
		close (_fdListen[i]);
	for (std::vector< Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close((*it)->getFd());
		delete (*it);
	}
	_clients.clear();
	close(_poll);
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
	}
}
