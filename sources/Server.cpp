#include "server.hpp"

volatile sig_atomic_t Server::flag = 0;
// volatile sig_atomic_t Server::nbServ = 0; //utile ?

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

void Server::launch()
{
	struct epoll_event event;
	struct sockaddr_in cli;
	socklen_t cli_len;
	int client_fd;
	char buff[MAXLINE];
	int n;
	int d;

	while(flag == 0) //on arrete avec ctrl+c voir pour recuperer le signal et libere tout proprement
	{
		// perror("right");
		d = epoll_wait(_poll, _events, SOMAXCONN, -1);
		for (int i = 0; i < d; i++)
		{
			if (_events[i].events & EPOLLIN)
			{
				if (_events[i].data.fd == _fdListen) // nouvelle connexion entrante pk ?
				{
					cli_len = sizeof(cli);
					client_fd = accept(_fdListen, (struct sockaddr *)&cli, &cli_len);
					if (client_fd < 0)
					{
						std::cout << "accept() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
						continue ; // imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
					}
					make_non_blocking(client_fd);
					event.data.fd = client_fd;
					event.events = EPOLLIN | EPOLLET;
					epoll_ctl(_poll, EPOLL_CTL_ADD, client_fd, &event);
					// add_client(client_fd, "", false);
					_clients.push_back(new Client(client_fd));
					std::cout << "Client connecté, fd= " << client_fd << std::endl; // a enlever
				}
				else // la socket n'est pas listenFd
				{
					client_fd = _events[i].data.fd;
					while (1)
					{
						n = read(client_fd, buff, sizeof(buff)); // faut il un -1 ici ? et donc un \0  apres ?
						if (n < 0)
						{
							if (errno == EAGAIN || errno == EWOULDBLOCK)
								break ; //plus rien a lire a voir ce qu on fait ici
							else //vraie erreur
							{
								close(client_fd);
								std::cout << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
								continue ; // imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
							}
						}
						else if (n == 0)
						{
							for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
							{
								if (client_fd == (*it)->getFd())
								{
									_clients.erase(it);
									break ;
								}
							}
							epoll_ctl(_poll, EPOLL_CTL_DEL, client_fd, NULL);
							close(client_fd);
							continue;
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
									}
									// break ; //doute ici
								}
							}
						}
					}
				}
				// if (_events[i].events & EPOLLOUT)
				// {
				// 	client_fd = _events[i].data.fd;
				// 	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
				// 	{
				// 		if (client_fd == (*it)->getFd())
				// 		{
				// 			n = send(client_fd, (*it)->getSendBuffer(), (*it)->setSendSize(), 0); //fonction a faire
				// 			if (n > 0)
				// 			{
				// 				(*it)->sendBuffErase(n); // a faire
				// 				if (((*it)->getToSend()).empty()) // tout a ete envoye
				// 					modifyEvent(client_fd, EPOLLIN);
				// 			}
				// 			else if // voir cas si -1 EAGAIN, ou 0
				// 		}
				// 	}
				// }

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
	close(_poll);
}

Server &Server::operator=(const Server &rhs)
{
	(void)rhs;
	return (*this);
}

// void Server::add_client(int fd, std::string str, bool d)
// {
// 	Client cli_data;
// 	cli_data.fd = fd;
// 	cli_data.buff = str;
// 	cli_data.request_complete = d;
// 	_clients.push_back(cli_data);
// }

void Server::handleSigint(int sig)
{
	if (sig == SIGINT)
	{
		std::cout << "serveur arrete proprement" << std::endl;
		flag = 1;
	}
}
