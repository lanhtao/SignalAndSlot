
#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_

#include <iostream>
#include <map>
#include <time.h>
#include "Signal.h"
#include "socketClient.h"
#include "ActiveObject.h"

typedef struct {
	char buf[128];
}message_t;
class MsgQueue : public ActiveObject
{
public:
	MsgQueue(std::string ipaddr, unsigned short port);
	virtual ~MsgQueue();

private:
	int run();
	void InitMsgQueue();
	int  StartQueue();
	int  StopQueue();
	void OnDataAvail(unsigned char* eRawData, int len);
	void OnConnectChange(int status);
	socketClient* 	m_client;
	std::string 	m_ipaddr;
	unsigned short 	m_port;
	bool			m_started;

};

#endif /* MSGQUEUE_H_ */
