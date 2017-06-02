/*
 * work.h
 *
 *  Created on: 2017-5-29
 *      Author: Administrator
 */

#ifndef WORK_H_
#define WORK_H_

#define CLIENTCOUNT 256
#define BODYBUF 1024

struct msg_t
{
	unsigned char head[4];
	char body[BODYBUF];
};

class work
{
public:
	work(int port);
	~work();

	void run();

private:
	void fix_socket_client(int index, int st);//将accept的客户端链接安装到socket_client的数组中
	void broadcast_user_status();
	void sendmsg(const struct msg_t *msg, ssize_t msglen);
	int auth_passwd(int uerid, const char *passwd);//验证用户密码
	void loginmsg(int st, int o_userid, const char *passwd);
	int setnonblocking(int st);//将socket设置为非阻塞
	int socket_accept();
	int socket_recv(int st);
	void user_logout(int st);
	int listen_st;
	int socket_client[CLIENTCOUNT];//声明socket_client数组，管理client的socket连接
};

#endif /* WORK_H_ */
