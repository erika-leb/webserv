#include "Server.hpp"

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
	event.events = EPOLLIN;
	// event.events = EPOLLIN | EPOLLET;
	epoll_ctl(_poll, EPOLL_CTL_ADD, client_fd, &event);
	for (std::map<int, ServerConfig>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	// for (std::map<int, ListenInfo>::iterator it = _fdListen.begin(); it != _fdListen.end(); it++)
	{
		// std::cout << "it->first = " << it->first << "; client fd = " << client_fd << std::endl;
		if (it->first == fd)
			_clients.push_back(new Client(client_fd, it->second));
	}
	// _clients.push_back(new Client(client_fd));
	std::cout << date(LOG) << ": Client(" << client_fd << ") connected" << std::endl;
}


void Server::prepareResponse(char buff[MAXLINE], std::string& tmp, int client_fd, Client *cli, int n)
{
	size_t endPos;
	Request *req;

	std::cout << date(LOG) << ": Request from client(" << client_fd << ")" << std::endl;
	// cli->addBuff(buff);
	cli->addBuff(buff, n);
	DEBUG_MSG("\nReceived: {\n" << cli->getBuff() << "}");

	if (cli->getRequest() == NULL && (cli->getBuff()).find("\r\n\r\n") != std::string::npos) // header complete so we create a new request
	{
		perror("angelito");
		req = new Request(*cli); // new request
		cli->setRequest(req); // we save the request in client
		DEBUG_MSG("scode mil parse = " << req->getsCode());
		req->parseHttp();
		endPos = cli->getBuff().find("\r\n\r\n") + 4; // we save the number of octet read after the header
		cli->setBodyRead(cli->getBuff().size() - endPos);
		cli->clearRequestBuff(0, 0);
		DEBUG_MSG("scodefin parse = " << req->getsCode());
		// cli->clearHeader(endPos);
	}
	else if (cli->getRequest() != NULL)
		cli->setBodyRead(cli->getBodyRead() + n); // if request was already created (= if there war already a header), we need to record the numeber of octet read (for the body)

	req = (cli->getRequest());

	// DEBUG_MSG("scode = " << req->getsCode());
	if (cli->getRequest() != NULL && req->parseBody() == true)
	// if (is_body_complete(cli) == true)
	// if (cli->getRequest() != NULL && req->getLenght() == cli->getBodyRead()) // the body is complete and can be procesed
	// this->_cgiHandler = req.getCgiHandler(_path.substr(_path.find_last_of(".")));
	{
		DEBUG_MSG("scode deb = " << req->getsCode());
		if ((req->getsCode() == 200) && req->is_cgi(req->getPathFile())) { // CGI cases
			cli->setCgi(new Cgi(*req, *cli));
			cli->getCgi()->handleCGI_fork(_poll);
			clearRequest(cli, req);
		}
		else if (req->getAction() != "POST" || req->getsCode() != 200) //GET, DELETE, ERROR cases
		{
			DEBUG_MSG("scode  sonar = " << req->getsCode());
			req->handleAction(req->getAction());
			tmp = req->makeResponse();
			modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
			clearRequest(cli, req);
		}
		else // POST with no error case
		{
			// req->parseBody();
			req->handleAction(req->getAction());
			tmp = req->makeResponse();
			modifyEvent(client_fd, EPOLLIN | EPOLLOUT);
			clearRequest(cli, req);
		}
	}
}

int Server::receiveRequest(int i, std::string& tmp)
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
				prepareResponse(buff, tmp, client_fd, (*it), n);
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

	(void) tmp;
	client_fd = _events[i].data.fd;
	for (std::vector<Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (client_fd == (*it)->getFd())
		{
			n = send(client_fd, (*it)->getSendBuffer(), (*it)->setSendSize(), 0);
			// std::cout << date(LOG) << ": Send " << n << " B to client(" << client_fd << ") [" << tmp << "]" << std::endl;
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



void Server::launch()
{
	struct epoll_event event;
	struct sockaddr_in cli;
	int d;
	std::string tmp; //revoir avec Thibault si a modifier

	while(flag == 0)
	{
		d = epoll_wait(_poll, _events, SOMAXCONN, timeOut());
		// std::cout << "DEBUG: Waiting for events..." << std::endl;
		// d = epoll_wait(_poll, _events, SOMAXCONN, 1000); // 1 seconde fixe pour tester
		// std::cout << "DEBUG: Events received: " << d << std::endl;
		for (int i = 0; i < d; i++)
		{
			if (_events[i].events & EPOLLIN)
			{
				if (is_listen_fd(_events[i].data.fd) == true)
					NewIncomingConnection(_events[i].data.fd, cli, event);
				else
				{
					// get the valid cgi and store it in a variable to reuse in ne next statement
					if (is_pipe_fd(_events[i].data.fd) == true) {
						if (receiveCgi(i, tmp) == 1)
							break;
					}
					else {
						if (receiveRequest(i, tmp) == 1)
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

