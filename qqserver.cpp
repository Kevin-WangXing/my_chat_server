//============================================================================
// Name        : qqserver.cpp
// Author      : wangxing
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "pub.h"

#include "work.h"

int main(int arg, char * args[])
{
	if(arg < 2)
	{
		printf("usage: qqserverd port\n");
		return -1;
	}
	int iport = atoi(args[1]);
	if(iport == 0)
	{
		printf("port %d is invalid\n", iport);
		return -1;
	}

	//setdaemon();

	work w(iport);


	printf("qqserver begin\n");

	signal1(SIGINT, catch_Signal);
	signal1(SIGPIPE, catch_Signal);

	w.run();
	printf("qqserver end\n");
	return 0;
}
