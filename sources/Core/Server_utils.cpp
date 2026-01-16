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
	int fd;

	// perror("ici");
	for (std::vector<Client *>::size_type i = 0; i < _clients.size(); i++)
	{
		// std::cout << "fd = " << _clients[i]->getFd() << std::endl;
		if (now - _clients[i]->getlastConn() > TIMEOUT_SECONDS)
		{
			std::cout << date(LOG) << ": Client(" << _clients[i]->getFd() << ") disconnected (TIMEOUT)" << std::endl;
			deleteSocket(_clients[i]->getFd());
		}
		if (_clients[i]->getCgi() != NULL && now - _clients[i]->getCgi()->getCgiTime() > CGI_TIMEOUT_SECONDS)
		{
			fd = _clients[i]->getCgi()->getFd(READ);
			std::cout << date(LOG) << ": Client(" << fd << ") disconnected (CGI TIMEOUT)" << std::endl;
			kill(_clients[i]->getCgiPid(), SIGINT);
			// epoll_ctl(_poll, EPOLL_CTL_DEL, fd, NULL);
			// close (fd);
			_clients[i]->getCgi()->setKilled(true);
			// delete (*(_clients[i]->getCgi()->getFd(READ))); // pointer freed
			// deleteSocket(_clients[i]->getCgi()->getFd(READ));
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
	// cli->clearRequestBuff(1); // ici changer pour s'arreter a la fin de la requete au cas ou on a deux requetes a la suite ???
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

int Server::receiveCgi( int i, int event ) {
	int pipe_fd, retval;

	pipe_fd = _events[i].data.fd;
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it)->getCgi() == NULL)
			continue;
		if (pipe_fd == (*it)->getCgi()->getFd(READ)) {
			retval = (*it)->getCgi()->handleCGI_pipe(pipe_fd, event);
			if (retval == 0) {
				epoll_ctl(_poll, EPOLL_CTL_DEL, pipe_fd, NULL);
				modifyEvent((*it)->getFd(), EPOLLIN | EPOLLOUT);
				(*it)->deleteCgi();
				DEBUG_MSG("CGI DELETED");
			}
		}
	}
	return retval;
}

void Server::insertPid( pid_t pid ) {
	_cgiPids.insert(pid);
}

void Server::removePid( pid_t pid ) {
	_cgiPids.erase(pid);
}


bool Server::is_chunk_complete( Client *cli )
{
	std::string::size_type end, pos;
	std::string size_str;
	std::stringstream ss;
	size_t size = 0;

	pos = 0;
	while (1)
	{
		// if (cli->getRequest()->getChunked() < 10)
		// 	std::cout << cli->getBuff() << std::endl;
		// cli->getRequest()->setChunked(cli->getRequest()->getChunked() + 1);
		end = cli->getBuff().find("\r\n", pos);
		if (end == std::string::npos)
		{
			DEBUG_MSG("ERROR3 = ");
			return (false);
		}

		// DEBUG_MSG("debu20 = " << cli->getBuff().substr(0, 20));
			// return (false);
		size_str = cli->getBuff().substr(pos, end - pos);
		// DEBUG_MSG("HEX = [" << size_str << "]");
		// DEBUG_MSG("HEX len = " << size_str.size());

		ss << std::hex << size_str;
		ss >> size;
		if (ss.fail())
		{
			cli->getRequest()->setCode(400);
			return (true); // ici on fait quoi ?
		}
		ss.clear();
		ss.str("");
		pos = end + 2;
		// DEBUG_MSG("size" << size);
		if (size == 0)
		{
			if (pos + 2 > cli->getBuff().size() || cli->getBuff().substr(pos, 2) != "\r\n")
			{
				DEBUG_MSG("ERROR2 = ");
				return (false);
			}
			break;
		}
		if (pos + size + 2 > cli->getBuff().size())
		{
			// DEBUG_MSG("ERROR1 = " << pos + size + 2 << " size =" << cli->getBuff().size());
			return (false);
		}
		// unsigned char c1 = cli->getBuff()[pos + size];
		// unsigned char c2 = cli->getBuff()[pos + size + 1];

		// DEBUG_MSG("EXPECT CRLF AFTER DATA");
		// DEBUG_MSG("pos=" << pos);
		// DEBUG_MSG("size=" << size);
		// DEBUG_MSG("byte1=" << (int)c1);
		// DEBUG_MSG("byte2=" << (int)c2);

		// _body << cli.getBuff().substr(pos, size);
		if (cli->getBuff().substr(pos + size, 2) != "\r\n") // changer ici pour 400 et true
		{
			DEBUG_MSG("ERROR = " << cli->getBuff().substr(pos + size, 2));
			DEBUG_MSG("ERROR = " << cli->getBuff().substr(pos + size - 10, 20));
			return (true);
		}
		pos += size + 2;
		// DEBUG_MSG("NEXT POS=" << pos);
		// DEBUG_MSG("NEXT 10 BYTES=[" << cli->getBuff().substr(pos, 10) << "]");
	}
	cli->getRequest()->checkLenght(pos + 2);
	return (true);
}

bool Server::is_body_complete( Client *cli )
{
	Request *req;
	std::string::size_type pos;

	// std::cout << "flag =" << cli->getRequest()->getExpect() << std::endl;
	if (cli->getRequest() == NULL)
		return (false);
	if (cli->getRequest()->getExpect() == 1) // si on a eu expect, il faut enter dans la boucle
		return (true);

	req = (cli->getRequest());
	if (req->getChunked() != 1  && req->getLenght() <= cli->getBodyRead()) // content-lenght body
		return (true);
	// if (req->getChunked() == 1) // chunked body
	// {
	// 	perror("rompiste");
	// 	return (is_chunk_complete(cli));
	// }
	if (req->getChunked() == 1)
	{
	    bool complete = is_chunk_complete(cli);
	    // std::cout << "Chunked check: " << cli->getBuff().size() << " bytes in buffer, Complete: " << complete << std::endl;
		pos = cli->getBuff().find("0\r\n\r\n");
		if  (pos != std::string::npos )
		{
	    	std::cout << "oui, pos =" << cli->getBuff().find("0\r\n\r\n") <<  std::endl;
			std::cout << "extrait =" << cli->getBuff().substr(pos - 2, pos + 6);
		}
		return complete;
	}
	return (false);
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



