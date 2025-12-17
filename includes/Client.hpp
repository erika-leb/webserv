#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "all.hpp"
#include "Cgi.hpp"
#include "ServerConfig.hpp"

class Cgi;

class ServerConfig;

class Request;

class Client {

	public:
		Client(int fd, ServerConfig &conf);
		Client(const Client &src);
		Client &operator=(const Client &src);
		~Client();

		int getFd() const;

		void addBuff(char *str, size_t len);
		std::string getBuff(void) const;
		const char *getSendBuffer();
		void sendBuffErase(int n);
		void setSendBuff(std::string str);
		void clearRequestBuff(void);

		std::string getToSend();
		size_t setSendSize();

		void addToSend();//temporaire

		void setCon( bool set );
		bool isCon();

		void setCgi( Cgi* newCgi );
		void deleteCgi(); // is this function really needed ?
		Cgi* getCgi();

		Request* getRequest() const;
		void setRequest(Request *req);
		void deleteRequest();

		void setlastConn(time_t);
		time_t getlastConn();

		ServerConfig &getServ();

		void setBodyRead(unsigned long long);
		unsigned long long	getBodyRead();

	private:
		Cgi* _cgi;
		Request *_req;

		int _fd;
		std::string _buff;
		std::string _sendBuff; //response to send
		bool _con;
		time_t _lastConn;
		ServerConfig &_conf;
		unsigned long long _bodyRead;

} ;

#endif
