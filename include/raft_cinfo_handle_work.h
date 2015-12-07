#ifndef RAFT_CINFO_HANDLE_WORK_H
#define RAFT_CINFO_HANDLE_WORK_H
/**
 * @file    raft_cinfo_handle_work.h
 * @brief   client info解析线程
 * @author  maguangxu
 * @version 1.0
 * @date    2015-09-08
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
#include "raft_lockfree_map.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftCInfoHandleWork : public Thread
{
public:
	//构造与析构
	 RaftCInfoHandleWork();
	~RaftCInfoHandleWork();

	//初始化函数
	static void Init(string ip, RaftGlobal::RaftParameter &para, int server_port);

	//友元类,用到本类中的客户端列表map
	friend class RaftCInfoSendWork;
	friend class RaftNotifyClientWork;

protected:
	//线程的启动栈帧
	virtual void run();

public:
	//对象指针
	static RaftCInfoHandleWork *s_raft_cinfo_handle_work_arr[RaftGlobal::skRaftCInfoHandleWorkNum];

private:
	//本机ip
	static string _ip;
	//raft配置参数对象
	static RaftGlobal::RaftParameter _raftparameter;
	//主服务端口
	static int _server_port;
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
	//follower间通信Socket
	static TCPSimpleClient _sock_arr[2][RaftGlobal::skFollowerMaxCount];
	//维护的客户端列表
	static mthread_map<std::string, uint64_t> _client_list;
	//时间戳
	struct timeval _tv;
};

}

#endif //RAFT_CINFO_HANDLE_WORK_H

// vim: ts=4 sw=4 nu

