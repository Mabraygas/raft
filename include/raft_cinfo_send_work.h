#ifndef RAFT_CINFO_SEND_WORK_H
#define RAFT_CINFO_SEND_WORK_H
/**
 * @file    raft_cinfo_send_work.h
 * @brief   client list发送线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-09-08
 */
#include <time.h>
#include <string>
#include <stdlib.h>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>
#include <eagle_simple_clientsocket.h>

#include <log.h>
#include "raft_global.h"
#include "raft_cinfo_handle_work.h"
#include "raft_lockfree_map.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftCInfoSendWork : public Thread
{
	
//构造与析构
public:
	RaftCInfoSendWork();
	~RaftCInfoSendWork();

	//初始化函数
	void Init();
protected:
	//线程的启动栈帧
	virtual void run();

public:
	//对象指针
	static RaftCInfoSendWork *s_raft_cinfo_send_work_arr[RaftGlobal::skRaftCInfoSendWorkNum];

private:
	//获取本机client列表并同步到其他follower
	void Do();
	//更新本机客户端列表
	void UpdateClientlist(const char* client_ip);
	//广播心跳函数
	int ClientlistBroadcast(const char* client_ip);
	//拼包函数
	int ConstructBroadcast(char* req_buf, const char* client_ip);
	
private:
	//system命令长度限制
	static const int skCmdLength = 1000;
	//clientlist发送时间戳
	struct timeval _tv, _now;

};
	
}

#endif //RAFT_CINFO_SEND_WORK_H

// vim: ts=4 sw=4 nu

