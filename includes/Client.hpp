#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "all.hpp"

class Client {

	public:
		Client(int fd);
		~Client();

		int getFd() const;
		void addBuff(char *str);
		std::string getBuff(void) const;

	private:
		int _fd;
		std::string _buff;

		Client();
		Client(const Client &src);
		Client &operator=(const Client &src);
} ;

#endif
