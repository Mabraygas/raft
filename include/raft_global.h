#ifndef RAFT_GLOBAL_H
#define RAFT_GLOBAL_H
/**
 * @file    raft_global.h
 * @brief   raft相关全局变量
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <string.h>
#include <stdint.h>
#include <urcu/uatomic.h>
#include <eagle_thread_queue.h>

namespace RAFT
{

using namespace std;
using namespace eagle;

//raft全局类
class RaftGlobal
{

public:
	//Follower的最大数量
	static const int skFollowerMaxCount = 64;
	//单例锁(getInstance前)、角色切换锁(getInstance后)
	static ThreadMutex _mutex;
	
	//Follower结构
	struct Follower
	{
		//follower的ip
		std::string ip;
		//follower的端口号
		int         port;
		//follower的唯一编号
		int         id;

		//默认构造函数
		Follower();
		//拷贝构造函数
		Follower(const Follower&);
		//赋值构造函数
		Follower& operator=(const Follower&);
		//重载运算符
		bool operator==(const Follower&);

	};

	//raft参数的结构体
	struct RaftParameter
	{
		//follower的数量
		int      follower_N;
		//follower列表
		Follower follower_list[skFollowerMaxCount];
		//强制Leader1(L1)
		string   forced_leader;
		//心跳间隔时间(ms)
		int      heartbeat_interval;
		//client接收Leader通知的端口号
		int      client_port;

		//默认构造函数
		RaftParameter();
		//拷贝构造函数
		RaftParameter(const RaftParameter&);
		//赋值构造函数
		RaftParameter& operator=(const RaftParameter&);
		//重载运算符
		bool operator==(const RaftParameter&);
		
	};

public:
	//raft心跳接收包结构体
	struct RaftRecvHeartBeatItem
	{
		uint32_t uid;
		//ip信息
		char ip[16];
		//sign for 心跳信息 or 客户端列表 0: 心跳 1: 客户端列表
		uint8_t client_list;
		//心跳信息与客户端列表的Union结构
		union {
			//心跳信息
			struct __attribute__ ((packed)) {
				//心跳类型
				int  type;
				//角色(F/C/L)
				char charactor;
				//阶段
				int  step;
			} HB_info;
		};
	} __attribute__ ((packed));
	//sizeof(RaftRecvHeartBeatItem) = 4+16+1+4+16 = 41(bytes)

	//心跳接收包结构初始化
	static void InitRecvHeartBeatItem(RaftRecvHeartBeatItem &);
	
	//raft心跳接收队列
	static ThreadQueue<RaftRecvHeartBeatItem> s_raft_recv_heartbeat_queue;

	//接收心跳包后, 根据本机的角色, 分别分发心跳包至以下三种队列中
	//Leader队列
	static ThreadQueue<RaftRecvHeartBeatItem> s_raft_leader_queue;
	//Candidate队列
	static ThreadQueue<RaftRecvHeartBeatItem> s_raft_candidate_queue;
	//Follower队列
	static ThreadQueue<RaftRecvHeartBeatItem> s_raft_follower_queue;

	//raft心跳发送包结构体
	struct RaftSendHeartBeatItem
	{
		uint32_t uid;
		//心跳类型
		int      type;
		//角色(F/C/L)
		char     charactor;
		//阶段
		int      step;
	} __attribute__ ((packed));

	//raft心跳发送队列
	static ThreadQueue<RaftSendHeartBeatItem> s_raft_send_heartbeat_queue;
	
public:
	//raft模块Follower之间的心跳版本
	static const uint8_t skRaftHeartBeatVer = 0x01;
	//raft模块Follower之间的心跳秘钥
	static const char*   skRaftHeartBeatKey;

	//Follower间, 心跳信息的四种类别
	enum
	{
		//心跳: 表明自身状态
		STATUS         = 0,
		//心跳: 拉票(广播)
		ASK_VOTE       = 1,
		//心跳: 投票
		VOTE           = 2,
		//心跳: 成为新任Leader(广播)
		UPGRADE_LEADER = 3,
	};

	//角色名
	static const char  FOLLOWER  = 'F';
	static const char  CANDIDATE = 'C';
	static const char  LEADER    = 'L';

public:
	//raft心跳接收线程个数
	static const int   skRaftRecvWorkNum   = 1;
	//raft心跳解析线程个数
	static const int   skRaftHandleWorkNum = 1;
	
	//raft leader工作线程个数
	static const int   skRaftLeaderWorkNum = 1;
	//raft Candidate工作线程个数
	static const int   skRaftCandidateWorkNum = 1;
	//raft follower工作线程个数
	static const int   skRaftFollowerWorkNum  = 1;
	
	//raft心跳发送线程个数
	static const int   skRaftSendWorkNum   = 1;
	//raft client info解析线程个数
	static const int   skRaftCInfoHandleWorkNum = 1;
	//raft client info发送线程个数
	static const int   skRaftCInfoSendWorkNum = 1;
	//raft通知Client心跳线程个数
	static const int   skRaftNotifyClientWorkNum = 1;
	//raft配置文件的存储位置
	static const char* skRaftConfigFile;

public:
	//Follower心跳Socket超时时间
	static const int   skFollowerSocketTimeout = 10; /* ms */
	//心跳包的最大长度(字节)
	static const int   skHeartBeatMaxLen = 100;
	//leader candiate follower工作线程扫描间隔时间
	static const int   skWorkInterval = 1; /* ms */
	
protected:
	//不能再类的外部创建类的实例
	RaftGlobal();
   ~RaftGlobal();
    //禁止复制
    RaftGlobal(const RaftGlobal&);
    //禁止赋值
	RaftGlobal& operator=(const RaftGlobal&);
};

}

#endif //RAFT_GLOBAL_H

// vim: ts=4 sw=4 nu
