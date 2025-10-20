#include "Server.hpp"

volatile sig_atomic_t Server::flag = 0;

Server::Server()
{
	struct sockaddr_in addr;
	struct epoll_event event;

	// nbServ++;\_
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

	make_non_blocking(_fdListen);
	_poll = epoll_create(1);
	event.data.fd = _fdListen;
	event.events = EPOLLIN | EPOLLET; //verifier pk
	epoll_ctl(_poll, EPOLL_CTL_ADD, _fdListen, &event);
}

void Server::modifyEvent(int fd, uint32_t events)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = events | EPOLLET;
	epoll_ctl(_poll, EPOLL_CTL_MOD, fd, &event);
}

void Server::deleteSocket(int client_fd)
{
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (client_fd == (*it)->getFd())
		{
			_clients.erase(it); //on enleve le pointeur de la liste
			delete (*it); //en free le pointeur
			break ;
		}
	}
	epoll_ctl(_poll, EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
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

	while(flag == 0) // faut penser a arreter ailleurs aussi
	{
		d = epoll_wait(_poll, _events, SOMAXCONN, -1);
		for (int i = 0; i < d; i++)
		{
			if (_events[i].events & EPOLLIN)
			{
				if (_events[i].data.fd == _fdListen) // new incoming connection
				{
					cli_len = sizeof(cli);
					client_fd = accept(_fdListen, (struct sockaddr *)&cli, &cli_len);
					if (client_fd < 0)
					{
						std::cout << "accept() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
						continue ;
					}
					make_non_blocking(client_fd);
					event.data.fd = client_fd;
					event.events = EPOLLIN | EPOLLET; //est-ce qu'on reste en EPOLLET ??
					epoll_ctl(_poll, EPOLL_CTL_ADD, client_fd, &event);
					_clients.push_back(new Client(client_fd));
					std::cout << "Client connecté, fd= " << client_fd << std::endl; // a enlever
				}
				else // the socket is not listenFd
				{
					client_fd = _events[i].data.fd;
					while (1)
					{
						n = read(client_fd, buff, sizeof(buff) - 1);
						if (n < 0)
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
								break ;
							else //vraie erreur
							{
								deleteSocket(client_fd);
								std::cout << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
								break ;
							}
						}
						else if (n == 0)
						{
							deleteSocket(client_fd);
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
									// it->buff.append(buff); //doute ici
									if (((*it)->getBuff()).find("\r\n\r\n") != std::string::npos)
									{
										//elle est complete faut traiter la demande
										// et renvoyer quelque chose
										Request req(*(*it));
										req.parseHttp();
										req.parseSpec();
										std::cout << req;
										modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
										(*it)->clearRequestBuff(); // erase the processed request
									}
									break ;
								}
							}
						}
					}
				}
			}
			if (_events[i].events & EPOLLOUT)
			{
				client_fd = _events[i].data.fd;
				for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
				{
					if (client_fd == (*it)->getFd())
					{
						while (1)
						{
							n = send(client_fd, (*it)->getSendBuffer(), (*it)->setSendSize(), 0);
							if (n > 0)
							{
								(*it)->sendBuffErase(n);
								if (((*it)->getToSend()).empty())
									modifyEvent(client_fd, EPOLLIN);
							}
							else if (n == 0)
							{
								deleteSocket(client_fd);
								break ;
							}
							else
							{
								if (errno == EAGAIN || errno == EWOULDBLOCK)
									break ;
								else // fatal error
								{
									deleteSocket(client_fd);
									std::cout << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
									break ;
								}
							}
						}
					}
					break ;
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
	// delete _clients;
	for (std::vector< Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
		// close((*it)->getFd());
		delete (*it);
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
		std::cout << "serveur arrete proprement" << std::endl;
		flag = 1;
	}
}
