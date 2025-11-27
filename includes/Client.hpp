#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "all.hpp"
#include "Cgi.hpp"

class Client {

	public:
		Client(int fd);
		Client(const Client &src);
		Client &operator=(const Client &src);
		~Client();

		int getFd() const;

		void addBuff(char *str);
		std::string getBuff(void) const;
		const char *getSendBuffer();

		std::string getToSend();
		void sendBuffErase(int n);
		size_t setSendSize();
		void setSendBuff(std::string str);

		void clearRequestBuff(void);

		void addToSend();//temporaire

		void setCon( bool set );
		bool isCon();

		void setCgi( Cgi* newCgi );
		Cgi* getCgi();

	private:
		Cgi* _cgi;

		int _fd;
		std::string _buff;
		std::string _sendBuff; //reponse a envoyer
		bool _con;

} ;

#endif
