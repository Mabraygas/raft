#include "raft_leader_work.h"

namespace RAFT
{

#define ABSTIME(tv) (uint64_t)((tv).tv_sec * 1000 + (tv).tv_usec / 1000)
//声明static变量
RaftLeaderWork* RaftLeaderWork::s_raft_leader_work_arr[RaftGlobal::skRaftLeaderWorkNum];

void RaftLeaderWork::Init()
{ }

RaftLeaderWork::RaftLeaderWork()
{ 
	//初始化Leader心跳时间戳
	gettimeofday(&_tv, NULL);
}

RaftLeaderWork::~RaftLeaderWork()
{ }

void RaftLeaderWork::run() {
	//接受元素
	RaftGlobal::RaftRecvHeartBeatItem recv_item;
	
	while(1) {
		//等待队列元素
		RaftGlobal::s_raft_leader_queue.pop_front(recv_item, RaftGlobal::skWorkInterval);

		//检查本机当前角色
		if(RaftGlobal::LEADER != RaftHandleWork::Get_Charactor()) { continue; }
		
		//获取现在时间, 看是否该发心跳了
		gettimeofday(&_now, NULL);
		if(ABSTIME(_now) - ABSTIME(_tv) > (uint64_t)RaftHandleWork::_raftparameter.heartbeat_interval) {
			//广播Leader心跳信息
			LeaderBroadcast();
			//更新最近心跳时间戳
			_tv = _now;
		}
	}
}

#define SOCK (RaftHandleWork::_sock_arr[curr_switch_no][sock_index])
int RaftLeaderWork::LeaderBroadcast() {
	//获取当前切换号
    int curr_switch_no = RaftHandleWork::GetCurrSwitchNo();

    //接收数据头部buf
    char   head_buf[9];
    int    ret;
    string err;
    
    //发送数据buf
    char   req_buf[RaftGlobal::skHeartBeatMaxLen];
    //接收数据bodybuf
    char   body_buf[RaftGlobal::skHeartBeatMaxLen];
    //拼包
    int packet_len = ConstructBroadcast(req_buf, RaftGlobal::STATUS, RaftGlobal::LEADER);
    
    for(int sock_index = 0; sock_index < RaftHandleWork::s_follower_num[curr_switch_no]; sock_index ++ ) {
        //不发送到本机
        if(RaftHandleWork::s_ip_arr[curr_switch_no][sock_index] == RaftHandleWork::_ip) {
            continue;
        }
        //尝试次数
        int try_num = 0;
        while(++try_num <= 1) {
            //发送
            ret = SOCK.send(req_buf, packet_len, err);
            //发送失败
            if(0 != ret) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " send leader heartbeat failed! try_num: " << try_num);
                continue;
            }
            
            memset(head_buf, 0x00, 9);
            //接收头部
            ret = SOCK.recvLength(head_buf, 9, err);
            //接收头部失败
            if(0 != ret) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " recv leader heartbeat head failed! try_num: " << try_num);
                continue;
            }

            //判断秘钥
            if(*(uint64_t *)RaftGlobal::skRaftHeartBeatKey != *(uint64_t *)head_buf) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " key err: " << *(uint64_t *)head_buf);
                SOCK.close();
                break;
            }

            //判断版本号
            if(RaftGlobal::skRaftHeartBeatVer != *(uint8_t *)(head_buf + 8)) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " ver err: " << *(uint8_t *)(head_buf + 8));
                SOCK.close();
                break;
            }

            //接收剩余数据
            ret = SOCK.recvLength(body_buf, 10, err);
            //接收剩余数据失败
            if(0 != ret) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " recv leader heartbeat body failed! try_num: " << try_num);
                continue;
            }

            //收发成功
            break;
		}
	}
	return 0;
}

int RaftLeaderWork::ConstructBroadcast(char* req_buf, int heartbeat_type, char charactor) {
 
    int offset = 0;
            
    *(uint64_t *)(req_buf + offset) = *(uint64_t *)RaftGlobal::skRaftHeartBeatKey;
    offset += sizeof(uint64_t);
                
    *(uint8_t *)(req_buf + offset)  = RaftGlobal::skRaftHeartBeatVer;
    offset += sizeof(uint8_t);

	*(uint8_t *)(req_buf + offset) = static_cast<uint8_t>(0);
	offset += sizeof(uint8_t);
            
    *(int *)(req_buf + offset) = heartbeat_type;
    offset += sizeof(int);

    *(char *)(req_buf + offset) = charactor;
    offset += sizeof(char);
            
    *(int *)(req_buf + offset) = RaftHandleWork::Get_Step();
    offset += sizeof(int);
                
    return offset;
}

}

// vim: ts=4 sw=4 nu

