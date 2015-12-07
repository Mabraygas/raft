#include "raft_cinfo_handle_work.h"

namespace RAFT
{

#define ABSTIME(tv) (uint64_t)((tv).tv_sec * 1000 + (tv).tv_usec / 1000)
	
//初始化static变量
RaftCInfoHandleWork* RaftCInfoHandleWork::s_raft_cinfo_handle_work_arr[RaftGlobal::skRaftCInfoHandleWorkNum];
string RaftCInfoHandleWork::_ip = "";
RaftGlobal::RaftParameter RaftCInfoHandleWork::_raftparameter;
int RaftCInfoHandleWork::_server_port = 0;
int RaftCInfoHandleWork::s_curr_switch_no = 0;
int RaftCInfoHandleWork::s_follower_num[2];
string RaftCInfoHandleWork::s_ip_arr[2][RaftGlobal::skFollowerMaxCount];
int RaftCInfoHandleWork::s_port_arr[2][RaftGlobal::skFollowerMaxCount];
int RaftCInfoHandleWork::s_id_arr[2][RaftGlobal::skFollowerMaxCount];
TCPSimpleClient RaftCInfoHandleWork::_sock_arr[2][RaftGlobal::skFollowerMaxCount];
mthread_map<std::string, uint64_t> RaftCInfoHandleWork::_client_list;

void RaftCInfoHandleWork::Init(string ip, RaftGlobal::RaftParameter &para, int server_port) {
	//初始化本机ip
	_ip = ip;
	//本机编号
	//int _id = 0;
	//初始化类的raft参数对象
	_raftparameter = para;
	//初始化主服务端口号
	_server_port = server_port;
	
	s_curr_switch_no = 0;
	s_follower_num[s_curr_switch_no] = _raftparameter.follower_N;
	for(int i = 0; i < s_follower_num[s_curr_switch_no]; i ++ ) {
		s_ip_arr[s_curr_switch_no][i]   = _raftparameter.follower_list[i].ip;
		s_port_arr[s_curr_switch_no][i] = _raftparameter.follower_list[i].port;
		s_id_arr[s_curr_switch_no][i]   = i;

		/*
		if(s_ip_arr[s_curr_switch_no][i] == _ip) {
			_id = s_id_arr[s_curr_switch_no][i];
		}
		*/
	}

	//建立Socket连接
	for(int i = 0; i < s_follower_num[s_curr_switch_no]; i ++ ) {
		//不建立本机连接
		if(s_ip_arr[s_curr_switch_no][i] == _ip) { continue; }
		//初始化socket
		_sock_arr[s_curr_switch_no][i].init(s_ip_arr[s_curr_switch_no][i], s_port_arr[s_curr_switch_no][i],
							RaftGlobal::skFollowerSocketTimeout, RaftGlobal::skFollowerSocketTimeout);
		//关闭Nagle算法
		_sock_arr[s_curr_switch_no][i].set_nodelay(true);
	}

}

RaftCInfoHandleWork::RaftCInfoHandleWork()
{ }

RaftCInfoHandleWork::~RaftCInfoHandleWork()
{ }

void RaftCInfoHandleWork::run() {
	//接收元素
	RaftGlobal::RaftRecvHeartBeatItem recv_item;
	bool bexist;

	while(1) {
		//初始化心跳接收包
		RaftGlobal::InitRecvHeartBeatItem(recv_item);

		//无限制等待
		bexist = RaftGlobal::s_raft_recv_clientlist_queue.pop_front(recv_item, -1);
		if(!bexist) { continue; } //不存在

		//获取传来的客户端ip
		std::string client_ip = string(recv_item.CL_list.client_ip, recv_item.CL_list.ip_length);

		//打时间戳
		gettimeofday(&_tv, NULL);

		uint64_t timestamp = ABSTIME(_tv);
		//更新客户端列表
		_client_list.update(client_ip, timestamp);
		
	}
}

}

// vim: ts=4 sw=4 nu

