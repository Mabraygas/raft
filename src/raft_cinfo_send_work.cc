#include "raft_cinfo_send_work.h"

namespace RAFT
{

#define ABSTIME(tv) (uint64_t)((tv).tv_sec * 1000 + (tv).tv_usec / 1000)
//声明static变量
RaftCInfoSendWork* RaftCInfoSendWork::s_raft_cinfo_send_work_arr[RaftGlobal::skRaftCInfoSendWorkNum];

void RaftCInfoSendWork::Init()
{ }

RaftCInfoSendWork::RaftCInfoSendWork()
{ 
	//初始化clientlist发送时间戳
	gettimeofday(&_tv, NULL);
}

RaftCInfoSendWork::~RaftCInfoSendWork()
{ }

void RaftCInfoSendWork::run() {
	
	while(1) {

		//获取现在时间, 看是否发送间隔超过阈值
		gettimeofday(&_now, NULL);
		if(ABSTIME(_now) - ABSTIME(_tv) > 10000 /* ms */) {
			//获取clientlist并发送
			Do();
			//更新最近心跳时间戳
			_tv = _now;
		}

		//sleep for a while
		struct timeval delay;
		delay.tv_sec  = RaftGlobal::skWorkInterval / 1000;
		delay.tv_usec = RaftGlobal::skWorkInterval - delay.tv_sec * 1000;
		select(0, NULL, NULL, NULL, &delay);

	}
}

void RaftCInfoSendWork::Do() {
	//拼shell命令
	char cmd[skCmdLength];
	snprintf(cmd, skCmdLength, "netstat -an | grep %d | grep ESTABLISHED | awk '{print $5}' | awk -F ':' '{print $1}' | sort | uniq", RaftCInfoHandleWork::_server_port);
	//捕获命令结果的缓冲区
	char resbuf[16 * RaftGlobal::skClientMaxCount];
	memset(resbuf, 0, sizeof(resbuf));

	FILE* fp = popen(cmd, "r");

	//捕获输出结果
	int bytes_count = fread(resbuf, 1, sizeof(resbuf), fp);
	if(!bytes_count) {
		WARN("System Call Error!");
	}
	if(bytes_count >= 16 * RaftGlobal::skClientMaxCount) {
		WARN("Client Num Too Many! Bigger than " << RaftGlobal::skClientMaxCount);
	}

	char *beg = resbuf, *end = strchr(resbuf, '\n');
	while(NULL != end) {
		//依次获取client的ip地址
		*end = '\0';
		//通过广播发送出去
		ClientlistBroadcast(beg);
		//更新本机的客户端列表
		UpdateClientlist(beg);
		//获取下一个ip
		beg = end + 1;
		end = strchr(resbuf, '\n');
	}
	
	pclose(fp);
}

void RaftCInfoSendWork::UpdateClientlist(const char* client_ip) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t timestamp = ABSTIME(tv);
	
	std::string s_client_ip = string(client_ip);
	RaftCInfoHandleWork::_client_list.update(s_client_ip, timestamp);
}

#define SOCK (RaftCInfoHandleWork::_sock_arr[curr_switch_no][sock_index])
int RaftCInfoSendWork::ClientlistBroadcast(const char* client_ip) {
	//获取当前切换号
    int curr_switch_no = RaftCInfoHandleWork::GetCurrSwitchNo();

    int    ret;
    string err;
    
    //发送数据buf
    char   req_buf[RaftGlobal::skHeartBeatMaxLen];
    //拼包
    int packet_len = ConstructBroadcast(req_buf, client_ip);
    
    for(int sock_index = 0; sock_index < RaftCInfoHandleWork::s_follower_num[curr_switch_no]; sock_index ++ ) {
        //不发送到本机
        if(RaftCInfoHandleWork::s_ip_arr[curr_switch_no][sock_index] == RaftCInfoHandleWork::_ip) {
            continue;
        }
        //尝试次数
        int try_num = 0;
        while(++try_num <= 1) {
            //发送
            ret = SOCK.send(req_buf, packet_len, err);
            //发送失败
            if(0 != ret) {
                ERROR(SOCK.getIp() << ":" << SOCK.getPort() << " send client list failed! try_num: " << try_num);
                continue;
            }
            
            //收发成功
            break;
		}
	}
	return 0;
}

int RaftCInfoSendWork::ConstructBroadcast(char* req_buf, const char* client_ip) {
 
    int offset = 0;
            
    *(uint64_t *)(req_buf + offset) = *(uint64_t *)RaftGlobal::skRaftHeartBeatKey;
    offset += sizeof(uint64_t);
                
    *(uint8_t *)(req_buf + offset)  = RaftGlobal::skRaftHeartBeatVer;
    offset += sizeof(uint8_t);

	*(uint8_t *)(req_buf + offset) = static_cast<uint8_t>(1);
	offset += sizeof(uint8_t);

	*(int *)(req_buf + offset) = strlen(client_ip);
	offset += sizeof(int);

	memcpy(req_buf + offset, client_ip, strlen(client_ip));
	offset += strlen(client_ip);
            
    return offset;
}

}

// vim: ts=4 sw=4 nu

