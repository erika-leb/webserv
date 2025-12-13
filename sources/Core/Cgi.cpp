#include "all.hpp"
#include "Cgi.hpp"

static int checkCode( std::string& str ) {
	trim_line(str);
	std::string code(str.substr(0, 3));
	std::stringstream ss(code);

	for (int i=0; i < 3; i++) {
		if (std::isdigit(code[i]) == 1) {
			str = "500 Internal server error";
			return 0;
		}
	}

	int icode;
	ss >> icode;
	if (icode < 100 || icode > 599) {
		str = "500 Internal server error";
		return 0;
	}
	return 0;
}

static std::string extractValue( std::string& line, std::string key ) {
	size_t pos = line.find(key);

	if (pos == std::string::npos) return "";
	
	size_t end = pos + key.length();
	while (line[end] != '\n' && line[end])
		end++;

	if (line[end - 1] == '\r')
		end--;
	return line.substr(pos + key.length(), end);
}

static std::string parseHeader( std::string& rawHeader, size_t cLen ) {
	std::stringstream ss;
	std::string tmp(extractValue(rawHeader, "Status: "));

	ss << "HTTP/1.1 ";
	if (tmp.length() == 0)
		ss << "200 Ok" << ENDLINE;
	else if ( checkCode(tmp) == 0)
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
	std::string tmp, content;

	std::getline(ss, tmp);
	while ( (tmp != "\n" || tmp != "\r\n") && (tmp.size() != 0)) {
		header.append(tmp);
		std::getline(ss, tmp);
	}
	while (std::getline(ss, tmp)) {
		content.append(tmp);	
	}

	header = parseHeader(header, content.size());
	return header + content;
}

Cgi::Cgi( std::string URI, std::string method, Client& cli ): _cli(cli), _path(URI), _method(method) {
	// Parse URI with query and stuff
	size_t pos = URI.find_first_of('?');
	if (pos != std::string::npos) {
		_path = URI.substr(0, pos);
		_queryString = URI.substr(pos + 1);
	}
	else
		_queryString = "";
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
	return *this;
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
	return -1;
}

void Cgi::makeEnv( std::vector<std::string> env_storage, std::vector<char *> envp) {
	std::map<std::string, std::string> cgi;

	cgi["REQUEST_METHOD"]  = _method;
    cgi["QUERY_STRING"]    = _queryString;
    cgi["SCRIPT_FILENAME"] = _path;
    cgi["CONTENT_LENGTH"]  = "0";
    cgi["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi["SERVER_SOFTWARE"] = "Webserv/1.0";
    cgi["SERVER_NAME"]     = "localhost"; // modify this according to configuration file don't know how
    cgi["SERVER_PORT"]     = "8080"; // Same as the other one

    env_storage.reserve(cgi.size());

    
    envp.reserve(cgi.size() + 1);

    for (std::map<std::string,std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it) {
        env_storage.push_back(it->first + "=" + it->second);
        envp.push_back(const_cast<char*>(env_storage.back().c_str()));
    }

    envp.push_back(NULL);
}

void Cgi::handleCGI_fork( int pollfd) {
	char * const args[] = {(char *)_path.c_str(), NULL};

	std::vector<std::string> tmp;
	std::vector<char *> env;
	makeEnv(tmp, env);

	if ( (pipe(_pipeDes)) == -1)
		DEBUG_MSG("PIPE ERROR"); // throw error
	
	if (fork() == 0) {
		// CHILD
		/* If POST request  or query dup2(STDIN, ?)*/
		close(_pipeDes[READ]);
		dup2(_pipeDes[WRITE], STDOUT_FILENO);
		_path = "./html/cgi/script.php";
		if ( (execve(_path.c_str(), args, &env[0])) == -1)
			DEBUG_MSG("EXECVE ERROR: " << errno << " (" << std::strerror(errno) << ")"); // throw error
		exit(1); // leak
	}
	else {
		// PARENT
		close(_pipeDes[WRITE]);
		if (make_non_blocking(_pipeDes[READ]) == -1) {
			std::cerr << "Non blocking error" << std::endl;
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
		buff[n] = '\0';
		std::string	strbuff(buff); // necessary ?
		ss << strbuff;
		
		strbuff = parseCgiOutput(ss);
		_cli.setSendBuff(strbuff);
		close(pipefd);
		return 0;
}

void Cgi::sigchld_handler( int sig ) {
	if (sig == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0);
	}
}