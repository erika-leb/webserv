#include "all.hpp"
#include "Cgi.hpp"

#define READ 0
#define WRITE 1

Cgi::Cgi( std::string URI, Client& cli ): _cli(cli) {
	// Parse URI with query and stuff
}

Cgi::Cgi( Cgi& cpy ): _cli(cpy._cli),  _path(cpy._path) {
	this->_pipeDes[0] = cpy._pipeDes[0];
	this->_pipeDes[1] = cpy._pipeDes[1];
}

Cgi& Cgi::operator=( Cgi& other ) {
	if (this != &other) {
		this->_cli = other._cli;
		this->_path = other._path;
		this->_pipeDes[0] = other._pipeDes[0];
		this->_pipeDes[1] = other._pipeDes[1];;
	}
}

Cgi::~Cgi() {}

void Cgi::handleCGI_fork( int pollfd, struct epoll_event events[SOMAXCONN] ) {

	if ( (pipe(_pipeDes)) == -1)
		; // throw error
	
	if (fork() == 0) {
		// CHILD
		/* If POST request dup2(STDIN, ?)*/
		close(_pipeDes[READ]);
		dup2(STDOUT_FILENO, _pipeDes[WRITE]);
		if ( (execve(_path.c_str(), NULL, NULL)) == -1)
			; // throw error
	}
	else {
		// PARENT
		close(_pipeDes[WRITE]);
	}
}

std::string Cgi::handleCGI_pipe( int pipefd ) {
		std::ostringstream	oss;
		char 				buff[MAXLINE];

		while ((read(pipefd, buff, MAXLINE)) > 0) {
			oss << buff;
		}
		close(pipefd);
		return oss.str();
}

void Cgi::sigchld_handler( int sig ) {
	if (sig == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0);
	}
}