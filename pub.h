/*
 * pub.h
 *
 *  Created on: 2017-5-29
 *      Author: Administrator
 */

#ifndef PUB_H_
#define PUB_H_


int signal1(int signo, void (*func)(int));
void catch_Signal(int Sign);
void setdaemon();

int socket_create(int port);


#endif /* PUB_H_ */
