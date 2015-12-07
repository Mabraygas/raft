#include "raft_receive_work.h"

namespace RAFT
{

void RaftReceiveWork::work(const RecvData &recv)
{
	
	//Raft�������հ�
	RaftGlobal::RaftRecvHeartBeatItem item;

	//���Raft�������հ���Ϣ
	item.uid       = recv.uid;
	
	memset(item.ip, 0x00, 16);
	memcpy(item.ip, recv.ip.c_str(), 15);

	item.client_list = *(uint8_t *)(recv.buffer.c_str());

	if(!item.client_list) { //�����������Ϣ
		
		item.HB_info.type 	   = *(int *)(recv.buffer.c_str() + 1);
		item.HB_info.charactor = *(char *)(recv.buffer.c_str() + 5);
		item.HB_info.step  	   = *(int *)(recv.buffer.c_str() + 6);

		//����raft�������ն���
		RaftGlobal::s_raft_recv_heartbeat_queue.push_back(item);
	}else { //����ǿͻ����б�
		
		item.CL_list.ip_length = *(int *)(recv.buffer.c_str() + 1);
		//�����������ݴ�����������, ��Ϊ��Parse�������Ѿ����˹���
		if(item.CL_list.ip_length < 0) { item.CL_list.ip_length = 0; }
		if(item.CL_list.ip_length > 15){ item.CL_list.ip_length = 15; }
		
		memcpy(item.CL_list.client_ip, recv.buffer.c_str() + 5, item.CL_list.ip_length);

		//����raft�ͻ����б���ն���
		RaftGlobal::s_raft_recv_clientlist_queue.push_back(item);
		
	}
}

}

// vim: ts=4 sw=4 nu
