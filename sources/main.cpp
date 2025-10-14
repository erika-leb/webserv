#include "server.hpp"

int make_non_blocking(int sockfd) //rendre une socket non bloquante
{
	int flag;

	flag = fcntl(sockfd, F_GETFL, 0);
	if (flag < 0)
	{
		std::cerr << "bind() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
		return (-1);
	}
	return (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK));
}


int is_fd_open(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) {
        if (errno == EBADF) {
			perror("fd ferme");
            // fd n'est pas ouvert
            return 0;
        }
    }
	perror("fd ouvert");
    // fd est ouvert
    return 1;
}


int initServ()
{
	struct sockaddr_in addr;
	struct sockaddr_in cli;
	socklen_t cli_len;
	int listen_fd;
	int client_fd;
	char buff[4096];
	int n;
	int d;
	struct epoll_event event, events[SOMAXCONN];
	int poll;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0); //AF_UNSPEC ne marche pas donc ici fonctionne que pour IPc4
	if (listen_fd == -1)
	throw std::runtime_error("socket() failed " + static_cast<std::string>(strerror(errno)));
	int optval = 1; // a enlever apres
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); //a enlever apres
	ft_memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8080);
	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr))) // utile car on est le serveur, inutile pour le client, equivaut a lister son numero dans l'annuaire
	{
		close(listen_fd);
		throw std::runtime_error("bind() failed " + static_cast<std::string>(strerror(errno)));
	}
	if(listen(listen_fd, SOMAXCONN) < 0)
	{
		close(listen_fd);
		throw std::runtime_error("listen() failed " + static_cast<std::string>(strerror(errno)));
	}
	make_non_blocking(listen_fd);
	poll = epoll_create(1);
	// perror("pain");
	event.data.fd = listen_fd;
	event.events = EPOLLIN | EPOLLET; //verifier pk
	epoll_ctl(poll, EPOLL_CTL_ADD, listen_fd, &event);
	// perror("cheval");

	while(1) //on arrete avec ctrl+c voir pour recuperer le signal et libere tout proprement
	{
		// perror("right");
		d = epoll_wait(poll, events, SOMAXCONN, -1);
		for (int i = 0; i < d; i++)
		{
			// perror("ici");
			if (events[i].data.fd == listen_fd) // nouvelle connexion entrante pk ?
			{
				cli_len = sizeof(cli);
				client_fd = accept(listen_fd, (struct sockaddr *)&cli, &cli_len);
				if (client_fd < 0)
				{
					std::cout << "accept() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
					continue ; // imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
				}
				make_non_blocking(client_fd);
				event.data.fd = client_fd;
				event.events = EPOLLIN | EPOLLET;
				epoll_ctl(poll, EPOLL_CTL_ADD, client_fd, &event);
				std::cout << "Client connecté, fd= " << client_fd << std::endl; // a enlever
			}
			else // la socket n'est pas listen_fd
			{
				// perror("wtf");
				client_fd = events[i].data.fd;
				n = read(client_fd, buff, sizeof(buff) - 1);
				if (n < 0 && errno != EAGAIN) //a voir ce cas,echec ou client deconnecte ?
				{
					close(client_fd);
					std::cout << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
					continue ; // imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
				}
				if (n == 0)
				{
					close(client_fd);
					continue;
				}
				buff[n] = '\0';
				std::cout << "Recu (" << n << " octets): " << std::endl;
				std::cout << buff << std::endl;
				// ici on parse la demande
				std::cout << "fd  = " << client_fd << std::endl;
				(is_fd_open(client_fd));
				// write(client_fd, "Message recu\n", 14); //ici on envoie la reponse
				// ou
				perror("dog");
				const char* resp = "HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 12\r\n"
				"\r\n"
				"Hello world!\n";
				// const char* resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 12\r\n\r\nHello world!\n";
				// perror("perra");
				// is_fd_open(client_fd);
				// std::cout << "resp = " << resp << std::endl;
				// fprintf(stderr, "dbg: client_fd=%d, resp=%p\n", client_fd, (void*)resp);
				// if (resp)
				//     fprintf(stderr, "dbg: preview resp (first 200 bytes): %.200s\n", resp);
				// else
				//     fprintf(stderr, "dbg: resp == NULL\n");

				// int rp = send(client_fd, resp, ft_strlen(resp), 0);
				// perror("the");
				// if (rp < 0)
				if (send(client_fd, resp, ft_strlen(resp), MSG_NOSIGNAL) < 0) // MSG empeche le sigpipe pas ouf
				{
					// perror("out");
					std::cout << "send() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
					// imprimer une erreur a l'ecran snas partir car ce n'est pas grave ?
				}
			}
			// perror("ls");
		}
		// perror("ici");

	}
	return (0);
}

int main()
{
	try
	{
		// Server serv();
		initServ();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

}
