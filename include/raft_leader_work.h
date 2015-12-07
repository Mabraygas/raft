#ifndef RAFT_LEADER_WORK_H
#define RAFT_LEADER_WORK_H
/**
 * @file    raft_leader_work.h
 * @brief   Leader工作线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-09-01
 */
#include <time.h>
#include <string>
#include <stdlib.h>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>
#include <eagle_simple_clientsocket.h>

#include <log.h>
#include "raft_global.h"
#include "raft_handle_work.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftLeaderWork : public Thread
{
	
//构造与析构
public:
	RaftLeaderWork();
	~RaftLeaderWork();

	//初始化函数
	void Init();
protected:
	//线程的启动栈帧
	virtual void run();

public:
	//对象指针
	static RaftLeaderWork *s_raft_leader_work_arr[RaftGlobal::skRaftLeaderWorkNum];

private:
	//广播心跳函数
	int LeaderBroadcast();
	//拼包函数
	int ConstructBroadcast(char* req_buf, int heartbeat_type, char charactor);
	
private:
	//leader心跳时间戳
	struct timeval _tv, _now;

};
	
}

#endif //RAFT_LEADER_WORK_H

// vim: ts=4 sw=4 nu

