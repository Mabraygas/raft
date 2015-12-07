#include "raft_notify_client_work.h"

namespace RAFT
{

EpollServerPtr RaftNotifyClientWork::s_server;
int RaftNotifyClientWork::_notify_port;

RaftNotifyClientWork* RaftNotifyClientWork::s_raft_notify_client_work_arr[RaftGlobal::skRaftNotifyClientWorkNum];

void RaftNotifyClientWork::Init(int notify_port) {
	_notify_port = notify_port;
}

RaftNotifyClientWork::RaftNotifyClientWork()
{ }

RaftNotifyClientWork::~RaftNotifyClientWork()
{ }

void RaftNotifyClientWork::run() {
	//NONE	
}

void RaftNotifyClientWork::Notify() {
	//获取所有客户端列表
	RaftCInfoHandleWork::_client_list.copy(_client_list);


}

size_t RaftNotifyClientWork::ConstructNotify(char* resp) {
	
    size_t offset = 0;
    
	//本机ip
	std::string ip = RaftCInfoHandleWork::_ip;

    *(uint64_t *)(resp + offset) = *(uint64_t *)RaftGlobal::skRaftHeartBeatKey;
    offset += sizeof(uint64_t);
                
    *(uint8_t *)(resp + offset)  = RaftGlobal::skRaftHeartBeatVer;
    offset += sizeof(uint8_t);
    
    *(int *)(resp + offset) = ip.length();
    offset += sizeof(int);
    
    memcpy(resp + offset, ip.c_str(), ip.length());
    offset += ip.length();
            
    return offset;
}

}

// vim: ts=4 sw=4 nu

