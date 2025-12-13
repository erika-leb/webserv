#include "Server.hpp"
#include "all.hpp"

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

void Server::modifyEvent(int fd, uint32_t events)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = events;
	epoll_ctl(_poll, EPOLL_CTL_MOD, fd, &event);
}

bool Server::is_listen_fd(int fd)
{
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
		if (it->first == fd)
			return (true);
	return false;
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


void Server::checkTimeOut()
{
	time_t now = std::time(NULL);

	// perror("ici");
	for (std::vector<Client *>::size_type i = 0; i < _clients.size(); i++)
	{
		// std::cout << "fd = " << _clients[i]->getFd() << std::endl;
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
		// std::cout << "fd = " << _clients[i]->getFd() << std::endl;
		elapsed = now - _clients[i]->getlastConn();
		if (TIMEOUT_SECONDS - elapsed > 0 && TIMEOUT_SECONDS - elapsed < res)
			res = TIMEOUT_SECONDS - elapsed;
	}
	return ((res + 2) * 1000);
}

void Server::clearRequest(Client *cli, Request *req)
{
  	delete req;
	cli->clearRequestBuff();
	cli->setRequest(NULL);
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



