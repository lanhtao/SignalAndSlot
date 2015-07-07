
#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include <netinet/tcp.h>
#include "ActiveObject.h"
#include "fcntl.h"
#include "Signal.h"

class socketClient : public ActiveObject
{
public:
	socketClient();
	virtual ~socketClient();
	Gallant::Signal2<unsigned char* , int > dataObserver_m ;
	Gallant::Signal1<int > ConnectObserver_m ;
	int openConnect(std::string ipaddr, int  port);
    int stopConnect();
	unsigned char SendMessage(const char *buf, int len);
private:
    int run ();
	int connectToServer();
    int checkAndConnectNetWork();
    void processMsgCallback(const char *object, size_t len);
    void ConnectStatusCallback(int status);

    int hostClientFd_m;
    pthread_mutex_t    write_mutex_m;        // 全局互斥锁对象
    pthread_mutex_t    startNet_mutex_m;      // 全局互斥锁对象
    std::string ipaddr_m;          // ip addr
    unsigned short port_m;         // port
};

#endif /* SOCKETCLIENT_H_ */
