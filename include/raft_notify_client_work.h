#ifndef RAFT_NOTIFY_CLIENT_WORK_H
#define RAFT_NOTIFY_CLIENT_WORK_H
/**
 * @file    raft_notify_client_work.h
 * @brief   raft通知客户端线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <map>
#include <string>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>

#include <log.h>
#include "raft_global.h"
#include "raft_cinfo_handle_work.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftNotifyClientWork : public Thread
{
public:
	//构造与析构
	 RaftNotifyClientWork();
	~RaftNotifyClientWork();

	//初始化函数
	void Init(int notify_port);

protected:
	//线程的启动栈帧
	virtual void run();

public:
	static RaftNotifyClientWork *s_raft_notify_client_work_arr[RaftGlobal::skRaftNotifyClientWorkNum];
	//新任Leader通知所有Client功能函数
	void Notify();
	
private:
	/**
	 * @brief 构造回复报文
	 *
	 * @param [resp]: 保存报文的缓冲区
	 *
	 * @return : 报文长度
	 */
	size_t ConstructNotify(char *resp);

public:
	//服务器指针
	static EpollServerPtr s_server;

private:
	//查询到的客户端列表
	map<std::string, uint64_t> _client_list;
	//客户端接收通知端口
	static int _notify_port;
};

}

#endif //RAFT_NOTIFY_CLIENT_WORK_H

// vim: ts=4 sw=4 nu

