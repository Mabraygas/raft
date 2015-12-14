#include "raft_handle_work.h"

namespace RAFT
{

//初始化static变量
RaftHandleWork* RaftHandleWork::s_raft_handle_work_arr[RaftGlobal::skRaftHandleWorkNum];
string RaftHandleWork::_ip = "";
RaftGlobal::RaftParameter RaftHandleWork::_raftparameter;
int RaftHandleWork::s_curr_switch_no = 0;
int RaftHandleWork::s_follower_num[2];
string RaftHandleWork::s_ip_arr[2][RaftGlobal::skFollowerMaxCount];
int RaftHandleWork::s_port_arr[2][RaftGlobal::skFollowerMaxCount];
int RaftHandleWork::s_id_arr[2][RaftGlobal::skFollowerMaxCount];
TCPSimpleClient RaftHandleWork::_sock_arr[2][RaftGlobal::skFollowerMaxCount];

char RaftHandleWork::_charactor;
int  RaftHandleWork::_step;
int  RaftHandleWork::_voted;
int  RaftHandleWork::_timewait;

void RaftHandleWork::Init(string ip, RaftGlobal::RaftParameter &para) {
	//初始化本机ip
	_ip = ip;
	//本机编号
	int _id = 0;
	//初始化类的raft参数对象
	_raftparameter = para;
	
	s_curr_switch_no = 0;
	s_follower_num[s_curr_switch_no] = _raftparameter.follower_N;
	for(int i = 0; i < s_follower_num[s_curr_switch_no]; i ++ ) {
		s_ip_arr[s_curr_switch_no][i]   = _raftparameter.follower_list[i].ip;
		s_port_arr[s_curr_switch_no][i] = _raftparameter.follower_list[i].port;
		s_id_arr[s_curr_switch_no][i]   = i;

		if(s_ip_arr[s_curr_switch_no][i] == _ip) {
			_id = s_id_arr[s_curr_switch_no][i];
		}
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

	//初始化开始阶段本机的角色及阶段
	if(_ip == _raftparameter.forced_leader) { //如果本机被配置为强制Leader
		_charactor = RaftGlobal::LEADER;
	}else {
		_charactor = RaftGlobal::FOLLOWER;
	}
	_step  = 1;
	_voted = 0;
	//随机种子的初始化
	srand(time(NULL));
	//每个Follower自身的心跳时限150 + (5-9) + 10*id ms
	_timewait = _raftparameter.heartbeat_interval + 10 * _id + (rand() % 5) + 5;
}

RaftHandleWork::RaftHandleWork()
{ }

RaftHandleWork::~RaftHandleWork()
{ }

void RaftHandleWork::run() {
	//接收元素
	RaftGlobal::RaftRecvHeartBeatItem recv_item;
	bool bexist;

	while(1) {
		//初始化心跳接收包
		RaftGlobal::InitRecvHeartBeatItem(recv_item);

		//无限制等待
		bexist = RaftGlobal::s_raft_recv_heartbeat_queue.pop_front(recv_item, -1);
		if(!bexist) { continue; } //不存在
		
		//分角色讨论处理方式
		
		//如果现在这台机器是Leader
		if(Get_Charactor() == RaftGlobal::LEADER) {
			//则检查收到的心跳包是否来自更高阶的Leader，如果是, 则本机降级为Follower
			if(recv_item.HB_info.charactor == RaftGlobal::LEADER && recv_item.HB_info.step > Get_Step()) {
				//降级
				INFO("Raft: Down to Follower. " << recv_item.ip << " step = " << recv_item.HB_info.step << " > _step = " << _step);
				Set_Charactor(RaftGlobal::FOLLOWER);
				Set_Step(recv_item.HB_info.step);
				Set_Voted(recv_item.HB_info.step);
				//推入Follower工作线程
				RaftGlobal::s_raft_follower_queue.push_back(recv_item);
			}
		}
		//如果现在这台机器是Follower
		else if(Get_Charactor() == RaftGlobal::FOLLOWER) {
			//直接推入Follower工作线程
			RaftGlobal::s_raft_follower_queue.push_back(recv_item);
		}
		//如果现在这台机器是Candidate
		else if(Get_Charactor() == RaftGlobal::CANDIDATE) {
			//先检查收到的心跳包是否来自某Leader或更高阶的Candidate, 如果是, 则本机降级为Follower
			if(recv_item.HB_info.charactor == RaftGlobal::LEADER || (recv_item.HB_info.charactor == RaftGlobal::CANDIDATE
					&& recv_item.HB_info.step > Get_Step())) {
				RaftGlobal::_mutex.lock();
				if(Get_Charactor() != RaftGlobal::CANDIDATE) {
					RaftGlobal::_mutex.unlock();
					continue;
				}
				//降级
				if(recv_item.HB_info.charactor == RaftGlobal::LEADER) {
					INFO("Raft: Down to Follower. Find Leader " << recv_item.ip);
				}else {
					INFO("Raft: Down to Follower. Find Candidate " << recv_item.ip << " with step = " << recv_item.HB_info.step << " > _step = " << Get_Step());
				}
				Set_Charactor(RaftGlobal::FOLLOWER);
				Set_Step(recv_item.HB_info.step);
				Set_Voted(recv_item.HB_info.step);
				//推入Follower工作线程
				RaftGlobal::s_raft_follower_queue.push_back(recv_item);
				RaftGlobal::_mutex.unlock();
			}
		}
	}
}

}

// vim: ts=4 sw=4 nu

