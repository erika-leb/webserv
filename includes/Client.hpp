#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "all.hpp"

class Client {

	public:
		Client(int fd, ListenInfo info);
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
		void setlastConn(time_t);
		time_t getlastConn();
		std::string &getIp();
		int getPort();

	private:
		int _fd;
		std::string _buff;
		std::string _sendBuff; //response to send
		bool _con;
		time_t _lastConn;
		std::string _ip;
		int 		_port;

} ;

#endif
