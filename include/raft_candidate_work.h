#ifndef RAFT_CANDIDATE_WORK_H
#define RAFT_CANDIDATE_WORK_H
/**
 * @file    raft_candidate_work.h
 * @brief   Candidate工作线程
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

class RaftCandidateWork : public Thread
{
	
//构造与析构
public:
	RaftCandidateWork();
	~RaftCandidateWork();

	//初始化函数
	void Init();
protected:
	//线程的启动栈帧
	virtual void run();

public:
	//对象指针
	static RaftCandidateWork *s_raft_candidate_work_arr[RaftGlobal::skRaftCandidateWorkNum];

private:
	//拉票函数
	int CandidateBroadcast();
	//拼包函数
	int ConstructBroadcast(char* req_buf, int heartbeat_type, char charactor);
	
private:
	//candidate拉票时间戳
	struct timeval _tv, _now;

};
	
}

#endif //RAFT_CANDIDATE_WORK_H

// vim: ts=4 sw=4 nu

