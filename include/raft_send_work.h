#ifndef RAFT_SEND_WORK_H
#define RAFT_SEND_WORK_H
/**
 * @file    raft_send_work.h
 * @brief   raft发送线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <string>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>

#include <log.h>
#include "raft_global.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftSendWork : public Thread
{
public:
	//构造与析构
	 RaftSendWork();
	~RaftSendWork();

protected:
	//线程的启动栈帧
	virtual void run();

public:
	static RaftSendWork *s_raft_send_work_arr[RaftGlobal::skRaftSendWorkNum];
	
private:
	/**
	 * @brief 构造回复报文
	 *
	 * @param [item]: 发送的数据 
	 * @param [resp]: 保存报文的缓冲区
	 *
	 * @return : 报文长度
	 */
	size_t ConstructResponse(const RaftGlobal::RaftSendHeartBeatItem &item, char *resp);

public:
	//服务器指针
	static EpollServerPtr s_server;
};

}

#endif //RAFT_SEND_WORK_H

// vim: ts=4 sw=4 nu

