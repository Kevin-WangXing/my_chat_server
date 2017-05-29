/*
 * pub.cpp
 *
 *  Created on: 2017-5-29
 *      Author: Administrator
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


void setdaemon()
{
	pid_t pid, sid;
	pid = fork();
	if(pid < 0)
	{
		printf("fork failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	if((sid = setsid()) < 0)//子进程调用setsid函数，将进程和控制台脱离关系
	{
		printf("setsid failed %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

