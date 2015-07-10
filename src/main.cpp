/*
 * main.cpp
 *
 *  Created on: Jul 6, 2015
 *      Author: lan
 */
#include "socketClient.h"
#include <iostream>
#include <stdlib.h>
#include "MsgQueue.h"

int main(int argc,char** argv)
{
	if(argc < 3)
	{
		std::cout<<"please input port and address\n"<<std::endl;
		exit(-1);
	}
	MsgQueue mq(argv[1],(unsigned short)atoi(argv[2]));
	while(1)
	{
		sleep(1);	
	};
	return 0;
}


