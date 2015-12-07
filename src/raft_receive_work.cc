#include "raft_receive_work.h"

namespace RAFT
{

void RaftReceiveWork::work(const RecvData &recv)
{
	
	//Raft心跳接收包
	RaftGlobal::RaftRecvHeartBeatItem item;

	//填充Raft心跳接收包信息
	item.uid       = recv.uid;
	
	memset(item.ip, 0x00, 16);
	memcpy(item.ip, recv.ip.c_str(), 15);

	item.client_list = *(uint8_t *)(recv.buffer.c_str());

	if(!item.client_list) { //如果是心跳信息
		
		item.HB_info.type 	   = *(int *)(recv.buffer.c_str() + 1);
		item.HB_info.charactor = *(char *)(recv.buffer.c_str() + 5);
		item.HB_info.step  	   = *(int *)(recv.buffer.c_str() + 6);

		//加入raft心跳接收队列
		RaftGlobal::s_raft_recv_heartbeat_queue.push_back(item);
	}else { //如果是客户端列表
		
		item.CL_list.ip_length = *(int *)(recv.buffer.c_str() + 1);
		//理论上以下容错处理不会起作用, 因为在Parse函数中已经做了过滤
		if(item.CL_list.ip_length < 0) { item.CL_list.ip_length = 0; }
		if(item.CL_list.ip_length > 15){ item.CL_list.ip_length = 15; }
		
		memcpy(item.CL_list.client_ip, recv.buffer.c_str() + 5, item.CL_list.ip_length);

		//加入raft客户端列表接收队列
		RaftGlobal::s_raft_recv_clientlist_queue.push_back(item);
		
	}
}

}

// vim: ts=4 sw=4 nu
