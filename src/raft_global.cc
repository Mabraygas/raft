#include "raft_global.h"

namespace RAFT
{
	
ThreadQueue<RaftGlobal::RaftRecvHeartBeatItem> RaftGlobal::s_raft_recv_heartbeat_queue;
ThreadQueue<RaftGlobal::RaftSendHeartBeatItem> RaftGlobal::s_raft_send_heartbeat_queue;

ThreadQueue<RaftGlobal::RaftRecvHeartBeatItem> RaftGlobal::s_raft_leader_queue;
ThreadQueue<RaftGlobal::RaftRecvHeartBeatItem> RaftGlobal::s_raft_candidate_queue;
ThreadQueue<RaftGlobal::RaftRecvHeartBeatItem> RaftGlobal::s_raft_follower_queue;
const char* RaftGlobal::skRaftConfigFile   = "raft.cfg";
const char* RaftGlobal::skRaftHeartBeatKey = "RAFTHKEY";

ThreadMutex RaftGlobal::_mutex;

//默认构造函数
RaftGlobal::Follower::Follower() {
	ip = "";
	port = 0;
	id = 0;
}

//拷贝构造函数
RaftGlobal::Follower::Follower(const Follower& another) {
	ip   = another.ip;
	port = another.port;
	id   = another.id;
}

//赋值构造函数
RaftGlobal::Follower& RaftGlobal::Follower::operator = (const Follower& another) {
	if(*this == another) {
		return *this;
	}
	this->ip   = another.ip;
	this->port = another.port;
	this->id   = another.id;

	return *this;
}

//重载运算符
bool RaftGlobal::Follower::operator == (const Follower& another) {
	return this->ip == another.ip && this->port == another.port && this->id == another.id;
}

//默认构造函数
RaftGlobal::RaftParameter::RaftParameter() {
	follower_N = 0;
	forced_leader = "";
	heartbeat_interval = 150;
	client_port = 7777;
}

//拷贝构造函数
RaftGlobal::RaftParameter::RaftParameter(const RaftParameter& another) {
	follower_N         = another.follower_N;
	
	for(int i = 0; i < follower_N; i ++ ) {
		follower_list[i] = another.follower_list[i];
	}
	
	forced_leader      = another.forced_leader;
	heartbeat_interval = another.heartbeat_interval;
	client_port        = another.client_port;
}

//赋值构造函数
RaftGlobal::RaftParameter& RaftGlobal::RaftParameter::operator = (const RaftParameter& another) {
	if(*this == another) {
		return *this;
	}
	this->follower_N         = another.follower_N;
	for(int i = 0; i < this->follower_N; i ++ ) {
		this->follower_list[i] = another.follower_list[i];
	}
	this->forced_leader      = another.forced_leader;
	this->heartbeat_interval = another.heartbeat_interval;
	this->client_port        = another.client_port;

	return *this;
}

//重载运算符
bool RaftGlobal::RaftParameter::operator == (const RaftParameter& another) {
	if(this->follower_N == another.follower_N && this->forced_leader == another.forced_leader
	 && this->heartbeat_interval == another.heartbeat_interval && this->client_port == another.client_port) {
		for(int i = 0; i < this->follower_N; i ++ ) {
			if(this->follower_list[i] == another.follower_list[i]) { continue; }
			else { return false; }
		}
		return true;
	}
	return false;
}

//初始化接收心跳包结构体
void RaftGlobal::InitRecvHeartBeatItem(RaftRecvHeartBeatItem& item) {
	memset((void *)&item, 0, sizeof(item));
}

}

// vim: ts=4 sw=4 nu
