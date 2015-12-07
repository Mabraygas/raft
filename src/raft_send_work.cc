#include "raft_send_work.h"

namespace RAFT
{

EpollServerPtr RaftSendWork::s_server;

RaftSendWork* RaftSendWork::s_raft_send_work_arr[RaftGlobal::skRaftSendWorkNum];

RaftSendWork::RaftSendWork()
{ }

RaftSendWork::~RaftSendWork()
{ }

size_t RaftSendWork::ConstructResponse(const RaftGlobal::RaftSendHeartBeatItem& item, char* resp) {
	size_t offset = 0;

	*(uint64_t *)(resp + offset) = *(uint64_t *)RaftGlobal::skRaftHeartBeatKey;
	offset += sizeof(uint64_t);

	*(uint8_t *)(resp + offset) = RaftGlobal::skRaftHeartBeatVer;
	offset += sizeof(uint8_t);

	*(uint8_t *)(resp + offset) = static_cast<uint8_t>(0);
	offset += sizeof(uint8_t);
	
	*(int *)(resp + offset) = item.type;
	offset += sizeof(int);

	*(char *)(resp + offset) = item.charactor;
	offset += sizeof(char);

	*(int *)(resp + offset) = item.step;
	offset += sizeof(int);

	return offset;
}

void RaftSendWork::run() {
	//发送元素
	RaftGlobal::RaftSendHeartBeatItem send_item;
	bool bexist;
	char resp_buf[RaftGlobal::skHeartBeatMaxLen];
	
	while(1) {
		//无限制等待
		bexist = RaftGlobal::s_raft_send_heartbeat_queue.pop_front(send_item, -1);
		//不存在
		if(!bexist) { continue; }

		memset(resp_buf, 0x00, RaftGlobal::skHeartBeatMaxLen);
		//拼包
		size_t packet_len = ConstructResponse(send_item, resp_buf);

		//回复
		s_server->send(send_item.uid, resp_buf, packet_len);
	} //while(1)
}
	
}

// vim: ts=4 sw=4 nu

