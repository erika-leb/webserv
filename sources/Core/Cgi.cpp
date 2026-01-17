#include "all.hpp"
#include "Cgi.hpp"

static int checkCode( std::string& str ) {
	trim_line(str);
	std::string code(str.substr(0, 3));
	std::stringstream ss(code);

	for (int i=0; i < 3; i++) {
		if (std::isdigit(code[i]) == 1) {
			str = "502 Bad gateway";
			return 0;
		}
	}

	int icode;
	ss >> icode;
	if (icode < 100 || icode > 599) {
		str = "502 Bad gateway";
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

	if (line[(pos + size) - 1] == '\r')
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
	this->_startTime = std::time(NULL);
	this->_killed = false;
	this->_pid = -1;
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

time_t	Cgi::getCgiTime()
{
	return (_startTime);
}

bool Cgi::getKilled()
{
	return (_killed);
}
void Cgi::setKilled(bool b)
{
	_killed = b;
}

void Cgi::makeEnv( std::vector<std::string>& env_storage, std::vector<char *>& envp) {
	std::map<std::string, std::string> cgi;

	cgi["REQUEST_METHOD"]  = _method;
    cgi["QUERY_STRING"]    = _queryString;

	// to get only the filenam and not the full path
	// size_t found = _path.find_last_of('/');
    // cgi["SCRIPT_FILENAME"] = _path.substr(found+1);

    cgi["SCRIPT_FILENAME"] = _path;

	// if (!_reqBody.empty())
	// 	cgi["CONTENT_LENGTH"]	= _reqBody.size();
	if (!_reqBody.empty()) {
		std::stringstream ss;
		ss << _reqBody.size();
		cgi["CONTENT_LENGTH"] = ss.str();
	} else {
		cgi["CONTENT_LENGTH"] = "0";
	}
	cgi["SERVER_PROTOCOL"] = "HTTP/1.1";
	cgi["GATEWAY_INTERFACE"] = "CGI/1.1";
	cgi["SERVER_SOFTWARE"] = "Webserv/1.0";
	cgi["SERVER_NAME"]	 = _serverName;
	cgi["SERVER_PORT"]	 = _port;

	env_storage.reserve(cgi.size());


	char *tmp;
	envp.reserve(cgi.size() + 1);
	for (std::map<std::string,std::string>::const_iterator it = cgi.begin(); it != cgi.end(); ++it) {
		env_storage.push_back(it->first + "=" + it->second);
		tmp = new char[env_storage.back().size() + 1];
		std::strcpy(tmp, env_storage.back().c_str());
		envp.push_back(tmp);
	}

	envp.push_back(NULL);
}

void Cgi::handleCGI_fork( int pollfd, Server& serv ) {
	char * const args[] = {(char *)_cgiHandler.c_str(), (char *)_path.c_str(), NULL};

	// for(std::vector<char *>::const_iterator it=env.begin(); it != env.end(); ++it) {
	// 	DEBUG_MSG("env: " << (*it));
	// }

	if ( (pipe(_pipeDes) == -1))
		throw std::runtime_error("pipe "+ static_cast<std::string>(std::strerror(errno)));

	int stdinPipe[2];
	if ( (pipe(stdinPipe) == -1)) {
		close(_pipeDes[WRITE]); close(_pipeDes[READ]);
		throw std::runtime_error("pipe "+ static_cast<std::string>(std::strerror(errno)));
	}

	_startTime = std::time(NULL);
	
	pid_t pid = fork();
	if (pid == -1) {
		close(stdinPipe[WRITE]); close(stdinPipe[READ]);
		close(_pipeDes[WRITE]); close(_pipeDes[READ]);
		throw std::runtime_error("fork "+ static_cast<std::string>(std::strerror(errno)));
	}

	if (pid == 0) {
		// CHILD
		/* If POST request dup2(STDIN, ?)*/
		std::vector<std::string> tmp;
		std::vector<char *> env;
		makeEnv(tmp, env);

		close(stdinPipe[WRITE]);
		dup2(stdinPipe[READ], STDIN_FILENO);
		close(stdinPipe[READ]);

		close(_pipeDes[READ]);
		dup2(_pipeDes[WRITE], STDOUT_FILENO);
		close(_pipeDes[WRITE]);

		execve(_cgiHandler.c_str(), args, &env[0]);
		// execve("bliblouy", args, &env[0]);
		std::string buff = "Status: 500 Internal server error\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html><html><head><style>h1 {text-align: center;}p {text-align: center;}div {text-align: center;}</style></head><body><h1>500</h1><div>Internal server error.</div></body></html>";
		std::cout << buff;

		for (std::vector<char *>::iterator it = env.begin(); it != env.end(); ++it) {
			if (*it)
				delete []*it;
		}


		throw std::runtime_error("execve " + static_cast<std::string>(std::strerror(errno)));
	}
	else {
		// PARENT
		serv.insertPid(pid);
		_cli.setCgiPid(pid);
		_pid = pid;

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

int Cgi::handleCGI_pipe( int pipefd, int event ) {
	char buff[MAXLINE];
	ssize_t n;

	// DEBUG_MSG("fac");
	if (_killed == true)
	{
		DEBUG_MSG("satis");
		close(pipefd);
		std::string output = "HTTP/1.1 504 Gateway Timeout\r\nContent-Type: text/html\r\nContent-Length: 154\r\nConnection: close\r\n\r\n<html><head><title>504 Gateway Timeout</title></head><body><center><h1>504 Gateway Timeout</h1><p>The CGI script took too long to respond.</p></center></body></html>";
		_cli.setSendBuff(output);
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0); // Nettoie le zombie après le kill
		return (0);
	}

	// if (event & (EPOLLHUP | EPOLLERR) && !(event & EPOLLIN)) {
	// 	close(pipefd);
	// 	std::string output(parseCgiOutput(_buff));
	// 	_cli.setSendBuff(output);
	// }

		if (event & (EPOLLHUP | EPOLLERR) && !(event & EPOLLIN)) {
		DEBUG_MSG("case");
		close(pipefd);
		int status;
    	// On vérifie si le fils a crashé
    	if (waitpid(_pid, &status, 0) != -1) {
    	    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    	        DEBUG_MSG("CGI crashed with status " << WEXITSTATUS(status));

				std::string body = "<html><body><h1>500 Internal Server Error</h1><p>CGI script failed.</p></body></html>";
				std::ostringstream resp;
				resp << "HTTP/1.1 500 Internal Server Error\r\n"
				     << "Content-Type: text/html\r\n"
				     << "Content-Length: " << body.size() << "\r\n"
				     << "Connection: close\r\n"
				     << "\r\n"
				     << body;

				_cli.setSendBuff(resp.str());

				return (0); // On sort, pas de parsing
    	    }
    	}
		std::string output(parseCgiOutput(_buff));
		DEBUG_MSG("output"<< output);
		_cli.setSendBuff(output);
	}

	if (event & EPOLLIN) {
		if ((n = read(pipefd, buff, sizeof(buff))) > 0) {
			buff[n] = '\0';
			_buff << buff;
			return 2;
		}
		else if (n < 0) {
			std::cerr << "HHHHread() failed: " << strerror(errno) << std::endl;
			close(pipefd);

			return 1;
		}
		else {
			close(pipefd);

						int status;
    		// On récupère le statut du processus associé à ce CGI
    		if (waitpid(_pid, &status, 0) != -1) {
    		    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
    		        // LE SCRIPT A CRASHÉ (ex: ton erreur Python while T)
    		        std::cerr << "CGI Error: script exited with status " << WEXITSTATUS(status) << std::endl;

					// std::string err = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: 100\r\nConnection: close\r\n\r\n<html><body><h1>500 Internal Server Error</h1><p>CGI script failed.</p></body></html>";
    		        // std::string err = "HTTP/1.1 500 Internal Server Error\r\n"
    		        //                           "Content-Type: text/html\r\n"
    		        //                           "Content-Length: 100\r\n\r\n"
    		        //                           "<html><body><h1>500 Internal Server Error</h1>"
    		        //                           "<p>CGI script failed.</p></body></html>";
					std::string body = "<html><body><h1>500 Internal Server Error</h1><p>CGI script failed.</p></body></html>";
					std::ostringstream resp;
					resp << "HTTP/1.1 500 Internal Server Error\r\n"
					     << "Content-Type: text/html\r\n"
					     << "Content-Length: " << body.size() << "\r\n"
					     << "Connection: close\r\n"
					     << "\r\n"
					     << body;

					_cli.setSendBuff(resp.str());
    		        // _cli.setSendBuff(err);
    		        return 0; // On s'arrête ici
    		    }
    		}

			std::string output(parseCgiOutput(_buff));
			_cli.setSendBuff(output);
		}
	}

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
	std::string ct(extractValue(rawHeader, "Content-Type: "));
	if (ct.empty())
		ct = "text/html";
	ss << "Content-Type: " << ct << ENDLINE;
	ss << "Content-Length: " << cLen << ENDLINE;
	ss << ENDLINE;

	return ss.str();
}

std::string Cgi::parseCgiOutput( std::stringstream& ss ) {
	std::string header;
	std::string tmp, content;

	std::getline(ss, tmp);
	tmp += '\n';
	while ( (tmp != "\n" && tmp != "\r\n") && (tmp.size() != 0)) {
		header.append(tmp);
		std::getline(ss, tmp);
		tmp += '\n';
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
		// while (waitpid(-1, NULL, WNOHANG) > 0);
	}
}
