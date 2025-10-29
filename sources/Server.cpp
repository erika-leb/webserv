#include "Server.hpp"

volatile sig_atomic_t Server::flag = 0;

Server::Server()
{
	struct sockaddr_in addr;
	struct epoll_event event;

	_fdListen = socket(AF_INET, SOCK_STREAM, 0);
	if (_fdListen == -1)
		throw std::runtime_error("socket() failed " + static_cast<std::string>(strerror(errno)));

	int optval = 1; // a enlever apres
	setsockopt(_fdListen, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); //a enlever apres

	ft_memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);

	if (bind(_fdListen, (struct sockaddr*)&addr, sizeof(addr))) // utile car on est le serveur, inutile pour le client, equivaut a lister son numero dans l'annuaire
	{
		close(_fdListen);
		throw std::runtime_error("bind() failed " + static_cast<std::string>(strerror(errno)));
	}

	if(listen(_fdListen, SOMAXCONN) < 0)
	{
		close(_fdListen);
		throw std::runtime_error("listen() failed " + static_cast<std::string>(strerror(errno)));
	}

	if (make_non_blocking(_fdListen) == -1)
		throw std::runtime_error("failed to set socket non-blocking " + static_cast<std::string>(strerror(errno)));
	_poll = epoll_create(1);
	event.data.fd = _fdListen;
	event.events = EPOLLIN;
	epoll_ctl(_poll, EPOLL_CTL_ADD, _fdListen, &event);
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
	// epoll_ctl(_poll, EPOLL_CTL_DEL, client_fd, NULL);
	// close(client_fd);
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (client_fd == (*it)->getFd())
		{
			epoll_ctl(_poll, EPOLL_CTL_DEL, client_fd, NULL);
			// perror("lala");
			// close(client_fd);  // MODIF ICI
			// perror("ouh");
			delete (*it); //en free le pointeur
			// perror("le");
			_clients.erase(it); //on enleve le pointeur de la liste
			// perror("cjat");
			break ;
		}
	}
	// perror("CHIENT");
}

void Server::launch()
{
	struct epoll_event event;
	struct sockaddr_in cli;
	socklen_t cli_len;
	int client_fd;
	char buff[MAXLINE];
	int n;
	int d;

	std::string tmp;
	int i = 0;

	while(flag == 0) // faut penser a arreter ailleurs aussi
	{
		// std::cout << "[DEBUG] _clients: " << std::endl;
		// for (std::vector<Client*>::iterator it=_clients.begin(); it < _clients.end(); ++it) {
		// 	std::cout << "client(" << (*it)->getFd() << ")" << std::endl;
		// }
		// std::cout << "[END]" << std::endl;
		d = epoll_wait(_poll, _events, SOMAXCONN, -1);
		i++;
		for (int i = 0; i < d; i++)
		{
			// printf("d = %d\n", d);
			// printf("event = %d\n", _events[i].events);
			// printf("EPOLLIN = %d\n", EPOLLIN);
			// printf("EPOLLOUT = %d\n", EPOLLOUT);
			if (_events[i].events & EPOLLIN) // EPOLLIN vaut 1
			{
				// perror("la");
				if (_events[i].data.fd == _fdListen) // new incoming connection
				{
					cli_len = sizeof(cli);
					client_fd = accept(_fdListen, (struct sockaddr *)&cli, &cli_len);
					if (client_fd < 0)
					{
						std::cout << "accept() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
						continue ;
					}
					if (make_non_blocking(client_fd) == -1)
					{
						close(client_fd);
						std::cerr << "failed to set client socket non-blocking" << std::endl;
						continue ;
					}
					event.data.fd = client_fd;
					event.events = EPOLLIN | EPOLLET; //est-ce qu'on reste en EPOLLET ??
					epoll_ctl(_poll, EPOLL_CTL_ADD, client_fd, &event);
					_clients.push_back(new Client(client_fd));
					std::cout << date(LOG) << ": Client(" << client_fd << ") connected" << std::endl;
				}
				else // the socket is not listenFd
				{
					// perror("baje");
					client_fd = _events[i].data.fd;
					n = read(client_fd, buff, sizeof(buff) - 1);
					// printf("n = %d i = %d, fd = %d\n", n, i, client_fd);
					if (n < 0)
					{
						deleteSocket(client_fd);
						std::cerr << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
						break ;
					}
					else if (n == 0)
					{
						std::cout << date(LOG) << ": Client(" << client_fd << ") disconnected (read() == 0)" << std::endl;
						deleteSocket(client_fd);
						// perror("verfi");
						break ;
					}
					else
					{
						buff[n] = '\0';
						for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
						{
							if (client_fd == (*it)->getFd())
							{
								(*it)->addBuff(buff);
								// std::cout << "Recu: " << (*it)->getBuff() << std::endl;
								if (((*it)->getBuff()).find("\r\n\r\n") != std::string::npos) //voir plus tard si on essaye de traiter la requete au fur et a mesure
								{
									Request req(*(*it));
									req.parseHttp();
									tmp = req.makeResponse(); // close or keep-alive depending on the value of connection
									modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
									(*it)->clearRequestBuff(); // erase the processed request
								}
								break ;
							}
						}
					}
				}
			}
			if (_events[i].events & EPOLLOUT) // EPOLLOUT vaut 4
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
							// perror("pss");
							// std::cout << "client " << client_fd << "closed" << std::endl; // a enlever
							std::cout << date(LOG) << ": Server closed connection to client(" << client_fd << ") [" << (*it)->isCon() << "]" << std::endl;
							// deleteSocket(client_fd);
							break ;
						}
						else
						{
							// perror("trop");
							deleteSocket(client_fd);
							std::cerr << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
							break ;
						}
						break ;
					}
				}
			}



				// buff[n] = '\0';
				// std::cout << "Recu (" << n << " octets): " << std::endl;
				// std::cout << buff << std::endl;
				// ici on parse la demande
				// const char* resp = "HTTP/1.1 200 OK\r\n"
				// "Content-Type: text/plain\r\n"
				// "Content-Length: 12\r\n"
				// "\r\n"
				// "Hello world!\n";
				// if (send(client_fd, resp, ft_strlen(resp), MSG_NOSIGNAL) < 0) // MSG empeche le sigpipe pas ouf
				// {
				// 	std::cout << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
				// 	// imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
				// }
		}
	}
}



Server::Server(const Server &src)
{
	(void) src;
}

Server::~Server()
{
	close (_fdListen);
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
		flag = 1;
	}
}
