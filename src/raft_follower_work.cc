#include "raft_follower_work.h"

namespace RAFT
{

#define ABSTIME(tv) (uint64_t)((tv).tv_sec * 1000 + (tv).tv_usec / 1000)
//声明static变量
RaftFollowerWork* RaftFollowerWork::s_raft_follower_work_arr[RaftGlobal::skRaftFollowerWorkNum];

void RaftFollowerWork::Init()
{ }

RaftFollowerWork::RaftFollowerWork()
{ 
	//初始化Leader心跳时间戳
	gettimeofday(&_tv, NULL);
}

RaftFollowerWork::~RaftFollowerWork()
{ }

void RaftFollowerWork::run() {
	//接受元素
	RaftGlobal::RaftRecvHeartBeatItem recv_item;
	//resp元素
	RaftGlobal::RaftSendHeartBeatItem send_item;
	
	bool bexist;
	
	while(1) {
		//等待队列元素
		bexist = RaftGlobal::s_raft_follower_queue.pop_front(recv_item, RaftGlobal::skWorkInterval);

		//检查本机当前角色
		if(RaftGlobal::FOLLOWER != RaftHandleWork::Get_Charactor()) { continue; }
		
		//如果没有元素出队
		if(!bexist) {
			//获取现在时间, 决定是否升级为Candidate
			gettimeofday(&_now, NULL);
			if(ABSTIME(_now) - ABSTIME(_tv) > (uint64_t)RaftHandleWork::Get_TimeWait()) {
				//升级为Candidate, 压入Candidate工作队列
				int _step = RaftHandleWork::Get_Step();
				RaftHandleWork::Set_Step(_step + 1);
				RaftHandleWork::Set_Charactor(RaftGlobal::CANDIDATE);
				
				INFO("Raft: Not recv Heartbeat in " << RaftHandleWork::Get_TimeWait() << " ms. Up to Candidate. step = " << RaftHandleWork::Get_Step());
			}
		}else { //有元素出队
			//更新时间戳
			gettimeofday(&_tv, NULL);
			//判断元素来自Leader(心跳)还是Candidate(拉票)
			if(RaftGlobal::LEADER == recv_item.HB_info.charactor) { //来自Leader心跳
				//检查step voted变量与Leader是否一致
				if(RaftHandleWork::Get_Step() != recv_item.HB_info.step) { RaftHandleWork::Set_Step(recv_item.HB_info.step); }
				if(RaftHandleWork::Get_Voted() != recv_item.HB_info.step) { RaftHandleWork::Set_Voted(recv_item.HB_info.step); }
				
				//回复心跳
				send_item.uid = recv_item.uid;
				PushSend(send_item, RaftGlobal::STATUS, RaftGlobal::FOLLOWER, RaftHandleWork::Get_Step());
			}
			else if(RaftGlobal::CANDIDATE == recv_item.HB_info.charactor && RaftGlobal::ASK_VOTE == recv_item.HB_info.type) { //来自Candidate的拉票请求
				INFO("Raft: Recv Candidate Ask Vote from " << recv_item.ip);
				if(recv_item.HB_info.step > RaftHandleWork::Get_Voted()) {
					INFO("Raft: Vote for Candidate " << recv_item.ip << " step = " << recv_item.HB_info.step);
					//投票给它
					send_item.uid = recv_item.uid;
					PushSend(send_item, RaftGlobal::VOTE, RaftGlobal::FOLLOWER, RaftHandleWork::Get_Step());
					RaftHandleWork::Set_Voted(recv_item.HB_info.step);
				}
			}
		}
	}
}

void RaftFollowerWork::PushSend(RaftGlobal::RaftSendHeartBeatItem& send_item, int type, char charactor, int step) {
    //填充发送元素
    
    send_item.type      = type;
    send_item.charactor = charactor;
    send_item.step      = step;
    //推入发送队列
    RaftGlobal::s_raft_send_heartbeat_queue.push_back(send_item);
}

}

// vim: ts=4 sw=4 nu

