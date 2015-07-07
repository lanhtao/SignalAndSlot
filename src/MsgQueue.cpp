
#include "MsgQueue.h"
#include <stdio.h>
MsgQueue::MsgQueue(std::string ipaddr, unsigned short port):ActiveObject("MsgQueue"),m_ipaddr(ipaddr),m_port(port)
{
	m_started = false;
	m_client = new socketClient();
	InitMsgQueue();
}

MsgQueue::~MsgQueue()
{

}

void MsgQueue::InitMsgQueue()
{
	m_client->dataObserver_m.Connect(this, &MsgQueue::OnDataAvail);  // subscription zigbee data
	m_client->ConnectObserver_m.Connect(this, &MsgQueue::OnConnectChange);  // subscription  connect change
	StartQueue();
}

void MsgQueue::OnDataAvail(unsigned char* buf, int len)
{
	for(int i = 0; i< len; i++)
	{
		printf("%x ",buf[i]);
	}
	printf("\n");
}

void MsgQueue::OnConnectChange(int status)
{
	printf("socket connect status:%d\n",status);
	if(status)
	{
		m_started = true;
	}else
	{
		m_started = false;
	}
}

int MsgQueue::StartQueue()
{
	active();
	return m_client->openConnect(m_ipaddr,m_port);
}

int MsgQueue::StopQueue()
{
	deactive();
	return m_client->stopConnect();
}

int MsgQueue::run ()
{

	return 0;
}



