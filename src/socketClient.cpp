#include "socketClient.h"
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <sstream>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>



socketClient::socketClient():ActiveObject("ZPort")
{
	hostClientFd_m = 0;
    ipaddr_m = "";          // ip addr
    port_m = 0;             // port
    pthread_mutex_init(&write_mutex_m, NULL);
    pthread_mutex_init(&startNet_mutex_m, NULL);
}


socketClient::~socketClient()
{

}

int socketClient::connectToServer()
{
	int conn_fd, flags, error;
	socklen_t len;
	int ret = -1;
	fd_set wset;
	struct timeval tval;
	struct sockaddr_in s_addr;

	for(int i=0; i<2; i++)
	{
		bzero(&s_addr,sizeof(struct sockaddr_in));
		s_addr.sin_family=AF_INET;
		s_addr.sin_addr.s_addr= inet_addr(ipaddr_m.c_str());  // covert host address name to stand numbers
		s_addr.sin_port=htons(port_m);

		// lan fd
		conn_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(-1 == conn_fd)
		{
			return -1;
		}
		flags = fcntl(conn_fd, F_GETFL, 0);
		fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK);
		if((ret = connect(conn_fd,(struct sockaddr *)(&s_addr), sizeof(struct sockaddr))) != 0)
		{
			// because set noblock so will return immediately error
			printf("socketClient:errno:%d\n",errno);
		}

		if(ret <= 0)
		{
			FD_ZERO(&wset);
			FD_SET(conn_fd, &wset);
			//rset = set;
			tval.tv_sec = 5;
			tval.tv_usec = 0;

			if((ret = select(conn_fd+1, NULL, &wset, NULL, &tval)) == 0)
			{
				errno = ETIMEDOUT;
				printf("socketClient:time out\n");
				ret = -1;
			}

			if(ret > 0)
			{
				if(FD_ISSET(conn_fd, &wset))
				{
					ret = 0;
					len = sizeof(error);
					if(getsockopt(conn_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
					{
						ret =-1;
					}
					printf("socketClient:errno:%d\n", error);
					if((ret == 0) && (error == 0))
					{
						hostClientFd_m = conn_fd;
						// set socket block
						flags = fcntl(hostClientFd_m, F_GETFL, 0);
						if(flags<0)
						{
							perror("fcntl(sock,GETFL)");
							exit(1);
						}
						flags &= ~O_NONBLOCK;
						if(fcntl(hostClientFd_m, F_SETFL, flags) < 0)
						{
							perror("fcntl(sock,SETFL,opts)");
							exit(1);
						}

						// set nodelay
						int on=1;
						setsockopt(hostClientFd_m, IPPROTO_TCP, TCP_NODELAY,&on,sizeof(on));

						ConnectStatusCallback(1);
						active();
						ret = 0;
						printf("socketClient:connect success\n");
						break;
					}else
					{
						ret=-1;
						close(conn_fd);
					}
				}
			}
			else
			{
				ret = -1;
			}
		}
	}

	return ret;
}


int socketClient::openConnect(std::string ipaddr, int  port)
{
	ipaddr_m = ipaddr;
	port_m = port;

	return checkAndConnectNetWork();
}

int socketClient::run ()
{
	static unsigned char buf[1500] = {0};
	int socket_ret = 0;
	int select_ret = 0;
	std::stringstream msg;
	struct timeval delay;
	delay.tv_sec = 5;
	delay.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(hostClientFd_m, &fds);

	select_ret = select(hostClientFd_m+1, &fds, NULL, NULL, &delay);
	 if( select_ret < 0)
	 {
		perror("socketClient:socket select error");
		close(hostClientFd_m);
		hostClientFd_m = 0;
		ConnectStatusCallback(0);
		deactive();
		//connectToServer();
	 }
	 else if(select_ret > 0)
	 {
		if (FD_ISSET(hostClientFd_m, &fds))
		{
			socket_ret = read(hostClientFd_m, buf, 1500);
			if (socket_ret > 0)
			{
				dataObserver_m.Emit(buf, socket_ret);//get data ,give slot did
			}
			else if(socket_ret == 0)   // connect closed
			{
				perror("socketClient:tcp socket connect closed\n");
				close(hostClientFd_m);
				hostClientFd_m = 0;
				ConnectStatusCallback(0);
				deactive();
				//connectToServer();
			}
			else if(socket_ret < 0)   // connect error
			{
				perror("socketClient:tcp socket read error\n");
				close(hostClientFd_m);
				hostClientFd_m = 0;
				ConnectStatusCallback(0);
				deactive();
				//connectToServer();
			}
		}
	 }

	return 0;
}

unsigned char socketClient::SendMessage(const char *buf, int len)
{
	unsigned char ret = 0;
    int i;
    int start = 0;
	pthread_mutex_lock(&write_mutex_m);            // lock the mutex 互斥锁上锁
		//printf("send data to host device len: %d\n", len);
		if(checkAndConnectNetWork() == 0)
		{
		    while (start < len)
		    {
				i = write (hostClientFd_m, buf + start, len-start);
				if (i <= 0)
				{
					ret = -1;
					break;
				}
				start += i;
		    }
		}
	pthread_mutex_unlock(&write_mutex_m);          // unlock the mutex解锁
	return ret;
}

int socketClient::checkAndConnectNetWork()
{
	int ret = -1;
	pthread_mutex_lock(&startNet_mutex_m);            // lock the mutex 互斥锁上锁
	if(hostClientFd_m == 0)
	{
		if(connectToServer()==0) // reconnection
		{
			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}
	pthread_mutex_unlock(&startNet_mutex_m);            // lock the mutex 解锁
	return ret;
}


// connect 1; disconnect 0
void socketClient::ConnectStatusCallback(int status)
{
	ConnectObserver_m.Emit(status);
}


int socketClient::stopConnect()
{
	deactive(); // stop network pthread
	return close(hostClientFd_m);
}


