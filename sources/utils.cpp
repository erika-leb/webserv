#include "Server.hpp"

void trim_line(std::string &line)
{
	size_t start = 0;
	size_t end = line.size();

	while (start < line.size() && std::isspace(line[start]))
		start++;
	while (end > start && std::isspace(line[end - 1]))
		end--;
	line = line.substr(start, end - start);
}

void	*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*temp;

	temp = (unsigned char *) b;
	while (len)
	{
		*(temp++) = (unsigned char) c;
		len--;
	}
	return (b);
}

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	if (!str)
		return (0);
	while (str[i])
		i++;
	return (i);
}

int make_non_blocking(int sockfd) //rendre une socket non bloquante
{
	int flag;

	flag = fcntl(sockfd, F_GETFL, 0);
	if (flag < 0)
	{
		std::cerr << "bind() failed " + static_cast<std::string>(strerror(errno)) << std::endl;
		return (-1); //verifier ce qu'on fait dans ce cas la
	}
	return (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK));
}

bool isBlank( char c ) {
	return std::isspace(static_cast<unsigned char>(c));
}

void remove_blank(std::string& str) {
	str.erase(std::remove_if(str.begin(), str.end(), isBlank), str.end());
}

std::string date(int format) {
	std::time_t date = std::time(0);
	std::tm* timezone;
	if (format == HTTP)
		timezone = std::gmtime(&date);
	else
		timezone = std::localtime(&date);

	char buff[80];
	switch (format)
	{
	case HTTP:
		std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", timezone);
		break;
	case LOG:
		// std::strftime(buff, sizeof(buff), "[%F][%H:%M:%S]", gmt);
		std::strftime(buff, sizeof(buff), "[%H:%M:%S]", timezone);
		break;

	default:
		buff[0] = '\0';
		break;
	}

	return std::string(buff);
}


bool isDirectivePresent(std::string name, std::vector<Directive> dir)
{
	for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
	{
		if (name == it->getName())
			return (true);
	}
	return (false);
}

Directive &getDirective(std::string name, std::vector<Directive> dir)
{
	for (std::vector<Directive>::iterator it = dir.begin(); it != dir.end(); ++it)
	{
		if (name == it->getName())
			return (*it);
	}
	throw std::runtime_error("root directive absent of conf file");
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
