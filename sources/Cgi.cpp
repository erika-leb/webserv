#include "all.hpp"
#include "Cgi.hpp"

static int checkCode( std::string& str ) {
	trim_line(str);
	std::string code(str.substr(0, 3));
	std::stringstream ss(code);

	int icode;
	ss >> icode;
	if (icode < 100 || icode > 599) {
		str = "500 Internal server error";
	}

}
static std::string extractValue( std::string& line, std::string key ) {
	size_t pos = line.find(key);

	if (pos == std::string::npos) return "";
	
	size_t end = pos + key.length();
	while (line[end] != '\n')
		end++;

	return line.substr(pos + key.length(), end);
}

static std::string parseHeader( std::string& rawHeader, size_t cLen ) {
	std::stringstream ss;
	std::string tmp(extractValue(rawHeader, "Status: "));

	ss << "HTTP/1.1 ";
	if (tmp.length() == 0)
		ss << "200 Ok" << ENDLINE;
	else
		ss << tmp << ENDLINE;
	ss << "Date: " << date(HTTP) << ENDLINE;
	ss << "Server: " <<  "localhost" << ENDLINE; // bruh we gonna struggle to get the server name there
	ss << "Content-Type: " << extractValue(rawHeader, "Content-Type: ") << ENDLINE;
	ss << "Content-Length: " << cLen << ENDLINE;
	ss << ENDLINE;

	return ss.str();
}

static std::string parseCgiOutput( std::stringstream& ss ) {
	std::string header;
	std::string tmp;

	while (tmp != "\n") {
		std::getline(ss, tmp);
		header += tmp;
		tmp.clear();
	}

	header = parseHeader(header, ss.str().size());
	return header + ss.str();
}

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

int Cgi::getFd( int fd ) {
	switch (fd) {
	case READ:
		return this->_pipeDes[READ];
	case WRITE:
		return this->_pipeDes[WRITE];
	default:
		break;
	}
}

void Cgi::handleCGI_fork( int pollfd) {

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
		if (make_non_blocking(_pipeDes[READ]) == -1) {
			std::cerr << "bruhhhhh" << std::endl;
			return ;
		}
		struct epoll_event event;
		event.data.fd = _pipeDes[READ];
		event.events = EPOLLIN;
		epoll_ctl(pollfd, EPOLL_CTL_ADD, _pipeDes[READ], &event);
	}
}

int Cgi::handleCGI_pipe( int pipefd ) {
		std::stringstream	ss;
		char 				buff[MAXLINE];

		int n = read(pipefd, buff, MAXLINE);
		if (n < 0) {
			std::cerr << "read() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
			return 1;
		}
		else if (n == 0) {
			return 1;
		}
		ss << buff;
		
		_cli.setSendBuff(parseCgiOutput(ss));
		close(pipefd);
}

void Cgi::sigchld_handler( int sig ) {
	if (sig == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0);
	}
}