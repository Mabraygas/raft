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
	}
}

}

// vim: ts=4 sw=4 nu
