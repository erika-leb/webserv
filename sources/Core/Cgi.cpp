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
	
	pos += key.length();
	size_t size = 0;
	for (size_t i=pos; line[i] != '\n' && line[i]; i++)
		size++;
	
	if (line[size - 1] == '\r')
		size--;

	return line.substr(pos, size);
}

Cgi::Cgi( Request& req, Client& cli ): _cli(cli) {

	// Parse URI with query and stuff
	std::string URI(req.getPathFile());

	this->_path = URI;
	this->_method = req.getAction();
	this->_serverName = req.getServIp(); _port = req.getServPort();

	size_t pos = URI.find_first_of('?');
	if (pos != std::string::npos) {
		this->_path = URI.substr(0, pos);
		this->_queryString = URI.substr(pos + 1);
	}
	else
		this->_queryString = "";

	this->_cgiHandler = req.getCgiHandler(_path.substr(_path.find_last_of(".")));
	this->_reqBody = req.getBody();
}

Cgi::Cgi( Cgi& cpy ):
	_cli(cpy._cli),  _path(cpy._path), _method(cpy._method),
		_queryString(cpy._queryString), _serverName(cpy._serverName), _port(cpy._port) {
	this->_pipeDes[0] = cpy._pipeDes[0];
	this->_pipeDes[1] = cpy._pipeDes[1];
}

Cgi& Cgi::operator=( Cgi& other ) {
	if (this != &other) {
		this->_cli = other._cli;
		this->_path = other._path;
		this->_method = other._method;
		this->_queryString = other._queryString;
		this->_serverName = other._serverName;
		this->_port = other._port;
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

	// to get only the filenam and not the full path
	// size_t found = _path.find_last_of('/');
    // cgi["SCRIPT_FILENAME"] = _path.substr(found+1);

    cgi["SCRIPT_FILENAME"] = _path;

    cgi["CONTENT_LENGTH"]  = "0";
    cgi["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi["SERVER_SOFTWARE"] = "Webserv/1.0";
    cgi["SERVER_NAME"]     = _serverName;
    cgi["SERVER_PORT"]     = _port;

    env_storage.reserve(cgi.size());

    
    envp.reserve(cgi.size() + 1);
    for (std::map<std::string,std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it) {
        env_storage.push_back(it->first + "=" + it->second);
        envp.push_back(const_cast<char*>(env_storage.back().c_str()));
    }

    envp.push_back(NULL);
}

void Cgi::handleCGI_fork( int pollfd ) {
	char * const args[] = {(char *)_cgiHandler.c_str(), (char *)_path.c_str(), NULL};

	std::vector<std::string> tmp;
	std::vector<char *> env;
	makeEnv(tmp, env);
	
	int stdinPipe[2];
	if ( (pipe(_pipeDes) == -1) || (pipe(stdinPipe) == -1))
		throw std::runtime_error("pipe "+ static_cast<std::string>(std::strerror(errno)));
	
	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("fork "+ static_cast<std::string>(std::strerror(errno)));
	
	if (pid == 0) {
		// CHILD
		/* If POST request dup2(STDIN, ?)*/
		close(stdinPipe[WRITE]);
		dup2(stdinPipe[READ], STDIN_FILENO);
		close(stdinPipe[READ]);

		close(_pipeDes[READ]);
		dup2(_pipeDes[WRITE], STDOUT_FILENO);
		close(_pipeDes[WRITE]);

		if ( (execve(_cgiHandler.c_str(), args, &env[0])) == -1) {
			// DEBUG_MSG("EXECVE ERROR: " << errno << " (" << std::strerror(errno) << ")"); // throw error
			throw std::runtime_error("execve "+ static_cast<std::string>(std::strerror(errno)));
		}
		exit(1); // leak
	}
	else {
		// PARENT

		close(stdinPipe[READ]);
        if (!_reqBody.empty()) {
            size_t total = _reqBody.size();
            size_t written = 0;
            const char *data = _reqBody.c_str();

            while (written < total)
            {
                ssize_t n = write(stdinPipe[WRITE], data + written, total - written);
                if (n <= 0)
                    break;
                written += n;
            }
        }
        close(stdinPipe[WRITE]); // EOF for CGI stdin

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
/*
int Cgi::handleCGI_pipe(int pipefd)
{
    std::string output;
    char buff[MAXLINE];
    ssize_t n;

    while ((n = read(pipefd, buff, sizeof(buff))) > 0)
    {
        output.append(buff, n);
    }

    if (n < 0)
    {
        std::cerr << "read() failed: " << strerror(errno) << std::endl;
        close(pipefd);
        return 1;
    }

    close(pipefd);

    output = parseCgiOutput(output);
    DEBUG_MSG("Send:\n" << output);
    _cli.setSendBuff(output);

    return 0;
}
*/

int Cgi::handleCGI_pipe( int pipefd ) {
		std::stringstream	ss;
		char 				buff[MAXLINE];

		int n = read(pipefd, buff, MAXLINE);
		if (n < 0) {
			std::cerr << "read() failed " + static_cast<std::string>(std::strerror(errno)) << std::endl;
			return 1;
		}
		else if (n == 0) {
			return 1;
		}
		buff[n] = '\0';
		std::string	strbuff(buff); // necessary ?
		ss << strbuff;
		
		strbuff = parseCgiOutput(ss);
		DEBUG_MSG("Send: \n" << strbuff);
		_cli.setSendBuff(strbuff);
		close(pipefd);
		return 0;
}

std::string Cgi::parseHeader( std::string& rawHeader, size_t cLen ) {
	std::stringstream ss;
	std::string tmp(extractValue(rawHeader, "Status: "));

	ss << "HTTP/1.1 ";
	if (tmp.length() == 0)
		ss << "200 Ok" << ENDLINE;
	else if ( checkCode(tmp) == 0)
		ss << tmp << ENDLINE;
	ss << "Date: " << date(HTTP) << ENDLINE;
	ss << "Server: " << _serverName + ":" << _port << ENDLINE;
	ss << "Content-Type: " << extractValue(rawHeader, "Content-Type: ") << ENDLINE;
	ss << "Content-Length: " << cLen << ENDLINE;
	ss << ENDLINE;

	return ss.str();
}

std::string Cgi::parseCgiOutput( std::stringstream& ss ) {
	std::string header;
	std::string tmp, content;

	std::getline(ss, tmp);
	while ( (tmp != "\n" || tmp != "\r\n") && (tmp.size() != 0)) {
		header.append(tmp + "\n");
		std::getline(ss, tmp);
	}

	while (std::getline(ss, tmp)) {
		content.append(tmp);	
	}

	/*
	std::istreambuf_iterator<char> iit(ss); std::istreambuf_iterator<char> eos;
	char prev;

	for (; iit != eos; ++iit) {
		if (*iit == '\n')
			break;
		header += *iit;
	}

	for (; iit != eos; ++iit) {
		content += *iit;
	}
	*/

	header = parseHeader(header, content.size());
	return header + content;

}

void Cgi::sigchld_handler( int sig ) {
	if (sig == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0);
	}
}