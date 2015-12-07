#ifndef RAFT_HANDLE_WORK_H
#define RAFT_HANDLE_WORK_H
/**
 * @file    raft_handle_work.h
 * @brief   raft解析线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <math.h>
#include <time.h>
#include <string>
#include <stdlib.h>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>
#include <eagle_simple_clientsocket.h>

#include <log.h>
#include <urcu/uatomic.h>
#include "raft_global.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftHandleWork : public Thread
{
public:
	//构造与析构
	 RaftHandleWork();
	~RaftHandleWork();

	//初始化函数
	static void Init(string ip, RaftGlobal::RaftParameter &para);

	//三个友元类
	friend class RaftLeaderWork;
	friend class RaftCandidateWork;
	friend class RaftFollowerWork;
	
protected:
	//线程的启动栈帧
	virtual void run();

public:
	//对象指针
	static RaftHandleWork *s_raft_handle_work_arr[RaftGlobal::skRaftHandleWorkNum];

private:
	//本机ip
	static string _ip;
	//raft配置参数对象
	static RaftGlobal::RaftParameter _raftparameter;
	//切换号, 用于切换Follower列表、配置时
	static int s_curr_switch_no;
	//Follower总数
	static int s_follower_num[2];
	//ip列表
	static string s_ip_arr[2][RaftGlobal::skFollowerMaxCount];
	//port列表
	static int s_port_arr[2][RaftGlobal::skFollowerMaxCount];
	//唯一编号id列表
	static int s_id_arr[2][RaftGlobal::skFollowerMaxCount];
	
	//获取当前切换号
	static int GetCurrSwitchNo() { return uatomic_read(&s_curr_switch_no); }
	//获取下次的切换号
	static int GetNextSwitchNo() { return !s_curr_switch_no; }
	//切换当前的切换号
	static void Switch() { uatomic_set(&s_curr_switch_no, !s_curr_switch_no); }
	//更新raft配置文件
	static void SwitchAll(RaftGlobal::RaftParameter _raftparameter);

private:
	//本机的角色(F/C/L)
	static char _charactor;
	//目前所处的阶段
	static int  _step;
	//已经给多大的阶段号Candidate投过票了
	static int  _voted;
	//本机等待Leader心跳的时限(时限内无心跳则变为Candidate)
	static int  _timewait;

public:
	//读写本机信息的一组函数
	static char Get_Charactor() 		{ return uatomic_read(&_charactor); }
	static void Set_Charactor(char des) { uatomic_set(&_charactor, des); }
	static int  Get_Step() 				{ return uatomic_read(&_step); }
	static void Set_Step(int des) 		{ uatomic_set(&_step, des); }
	static int  Get_Voted() 			{ return uatomic_read(&_voted); }
	static void Set_Voted(int des)		{ uatomic_set(&_voted, des); }
	static int  Get_TimeWait() 			{ return uatomic_read(&_timewait); }
	static void Set_TimeWait(int des) 	{ uatomic_set(&_timewait, des); }


private:
	static TCPSimpleClient _sock_arr[2][RaftGlobal::skFollowerMaxCount];
	
};

}

#endif //RAFT_HANDLE_WORK_H

// vim: ts=4 sw=4 nu

