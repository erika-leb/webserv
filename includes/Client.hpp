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
		const char *getSendBuffer();
		std::string getToSend();
		void sendBuffErase(int n);
		size_t setSendSize();
		void clearRequestBuff(void);

		void addToSend();//temporaire

	private:
		int _fd;
		std::string _buff;
		std::string _sendBuff; //reponse a envoyer

		Client();
		Client(const Client &src);
		Client &operator=(const Client &src);
} ;

#endif
