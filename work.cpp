/*
 * work.cpp
 *
 *  Created on: 2017-5-29
 *      Author: Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "work.h"
#include "pub.h"

int work::setnonblocking(int st)
{
	int opts = fcntl(st, F_GETFL);
	if(opts < 0)
	{
		printf("fcntl failed %s\n", strerror(errno));
		return 0;
	}
	opts = opts | O_NONBLOCK;
	if(fcntl(st, F_SETFL, opts) < 0)
	{
		printf("fcntl failed %s\n", strerror(errno));
		return 0;
	}
	return 1;
}

int work::socket_accept()
{
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, sizeof(client_addr));
	int client_st = accept(listen_st, (struct sockaddr *)&client_addr, &len);//接受来自client的socket连接

	if(client_st < 0)
	{
		printf("accept filed %s\n", strerror(errno));
	}else
		printf("accept by %s\n", inet_ntoa(client_addr.sin_addr));
	return client_st;//返回来自client端的socket描述符
}

//将accept的客户端链接安装到socket_clinet【】数组中，index为client登录时的userID，st为client socket
void work::fix_socket_client(int index, int st)
{
	if(socket_client[index] != 0)//同一个userID没有下线，却又在另一个终端登录，拒绝登录
	{
		printf("%d:userid already login\n", index);
		struct msg_t msg;
		memset(&msg, 0,sizeof(msg));
		msg.head[0] = 2;//系统消息
		msg.head[1] = 3;//userid已经登陆
		msg.head[2] = 0;//暂时保留
		msg.head[3] = 0;//
		send(st, (char *)&msg, sizeof(msg.head), 0);//给client端socket下发系统消息
		close(st);
	}else
	{
		//如果socket_client【index]等于0，将client端socket赋予socket_client【index]
		socket_client[index] = st;
	}
}

//向socket_client数组中的所有client广播用户状态消息
void work::broadcast_user_status()
{
	struct msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.head[0] = 1;//设置用户消息类型Wie用户状态消息

	for(int i = 0; i < CLIENTCOUNT; i++)//设置用户状态消息体
	{
		if(socket_client[i] != 0)
		{
			msg.body[i] = '1';//在线
		}else
		{
			msg.body[i] = '0';//离线
		}
	}

	for(int i = 0; i < CLIENTCOUNT; i++)//向socket_client数组中每个client广播用户状态消息
	{
		if(socket_client[i] != 0)
		{
			send(socket_client[i], &msg, strlen(msg.body) + sizeof(msg.head), 0);
			printf("%d:broadcast %s\n", i, msg.body);
		}
	}
}

int work::auth_passwd(int userid, const char *passwd)//验证用户密码
{
	if(strncmp(passwd, "", 1) == 0)
		return 1;
	else
		return 0;//密码验证失败
}

void work::loginmsg(int st, int o_userid, const char *passwd)
{
	struct msg_t msg;
	memset(&msg, 0, sizeof(msg));
	msg.head[0] = 2;//系统消息
	msg.head[2] = 0;//暂时保留，填0
	msg.head[3] = 0;//暂时保留，填0

	if((o_userid < 0) || (o_userid >= CLIENTCOUNT))//无效的userid
	{
		printf("login failed, %d:invalid userid\n", o_userid);
		msg.head[1] = 1;//无效userid
		send(st, (const char *)&msg, sizeof(msg.head), 0);
		close(st);
		return;
	}

	if(!auth_passwd(o_userid, passwd))//验证用户登录userID和密码
	{
		printf("login failed, userid = %d, passwd = %s:invalid passwd\n", o_userid, passwd);
		msg.head[1] = 2;//无效密码
		//给client端socket下发系统消息
		send(st, (const char *)&msg, sizeof(msg.head), 0);
		close(st);//验证失败，关闭client socket，函数返回
		return;
	}

	printf("%d:login success\n", o_userid);
	fix_socket_client(o_userid, st);
	broadcast_user_status();//
}

void work::sendmsg(const msg_t * msg, ssize_t msglen)//处理send消息
{
	if((msg->head[2] < 0) || msg->head[2] >= CLIENTCOUNT)    //没有这个目标用户
	{
		printf("%d: have not this userid\n", msg->head[2]);
	}else
	{
		if(socket_client[msg->head[2]] == 0)   //目的user不在线
		{
			printf("%d:userid not online\n", msg->head[2]);
		}else
		{
			//给client端socket下发send消息
			send(socket_client[msg->head[2]], (const char *)msg, msglen, 0);
			printf("send message:%d to %d-%s\n", msg->head[1], msg->head[2], msg->body);
		}
	}
}

int work::socket_recv(int st)
{
	struct msg_t msg;
	memset(&msg, 0, sizeof(msg));
	ssize_t rc = recv(st, (char *)&msg, sizeof(msg), 0);//接受来自client socket发送来的消息

	printf("msg.head[0] = %d\n", msg.head[0]);
	printf("msg.head[1] = %d\n", msg.head[1]);
	printf("msg.head[2] = %d\n", msg.head[2]);
	printf("msg.head[3] = %d\n", msg.head[3]);
	printf("msg.body = %s\n", msg.body);

	if(rc <= 0)//接受失败
	{
		if(rc < 0)
			printf("recv failed %s\n", strerror(errno));
	}else
	{
		switch(msg.head[0])
		{
		case 0:  //send 消息
			sendmsg(&msg, rc);
			break;
		case 1:  //login消息
			loginmsg(st, msg.head[1], msg.body);
			break;
		default://无法识别的消息
			printf("login failed, it's not login message %s\n", (const char *)&msg);
			msg.head[0] = 2;//系统消息
			msg.head[1] = 0;//无法识别的消息
			msg.head[2] = 0;//暂时保留，填0
			msg.head[3] = 0;//暂时保留，填0
			send(st, (const char *)&msg, sizeof(msg.head), 0);//给client端下发系统消息
			return 0;
		}
	}
	return rc;
}

void work::user_logout(int st)//client socket 连接断开
{
	for(int i = 0; i < CLIENTCOUNT; i++)//循环遍历socket_client【】数组
	{
		if(socket_client[i] == st)//找到socket_client【】数组中与相等的client socket
		{
			printf("userid = %d socket disconnected\n", i);
			close(socket_client[i]);//关闭socket——client【】数组中相应的socket
			socket_client[i] = 0;//将数组socket_clinet【】 中相应的元素初始化为0，以便该userID下次还可以继续登录
			broadcast_user_status();
			return;
		}
	}
}

work::work(int port)
{
	memset(socket_client, 0, sizeof(socket_client));
	listen_st = socket_create(port);//创建server端socket
	if(listen_st == 0)
		exit(-1);//创建socket失败，程序退出
}

work::~work()
{

}

void work::run()
{
	//声明epoll_event结构体的变量，ev用于注册事件，数组用于回传要处理的事件
	struct epoll_event ev, events[CLIENTCOUNT];
	setnonblocking(listen_st);//设置服务端listen的socket为非阻塞
	int epfd = epoll_create(CLIENTCOUNT);
	ev.data.fd = listen_st;//将listen_st放入epoll
	ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_st, &ev);

	int st = 0;
	while(1)
	{
		//epoll_wait阻塞，直到epoll当中的socket有指定的消息产生
		int nfds = epoll_wait(epfd, events, CLIENTCOUNT, -1);
		if(nfds == -1)
		{
			printf("epoll_wait failed %s\n", strerror(errno));
			break;
		}
		for(int i = 0; i < nfds; i++)//循环处理epoll_wait返回产生所有出事的socket
		{
			if(events[i].data.fd < 0)
				continue;

			if(events[i].data.fd == listen_st)//代表listen_st收到来自客户端的connect
			{
				st = socket_accept();
				if(st >= 0)
				{
					setnonblocking(st);
					ev.data.fd = st;
					ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
					epoll_ctl(epfd, EPOLL_CTL_ADD, st, &ev);
					continue;
				}
			}

			if(events[i].events & EPOLLIN)//有来自client的数据
			{
				st = events[i].data.fd;//得到有效的数据socket
				if(socket_recv(st) <= 0)
				{
					user_logout(st);
					events[i].data.fd = -1;
				}
			}
			if(events[i].events & EPOLLERR)//有来自client出错
			{
				st = events[i].data.fd;//得到有数据的socket
				user_logout(st);
				events[i].data.fd = -1;

			}

			if(events[i].events & EPOLLHUP)//有来自client关闭
			{
				st = events[i].data.fd;//得到有数据的socket
				user_logout(st);
				events[i].data.fd = -1;
			}

		}
	}
	close(epfd);
}
