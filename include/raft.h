#ifndef RAFT_H
#define RAFT_H

/**
 * @file 	raft.h
 * @brief   自选举模块
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-26
 */

#include <string>
#include <stdint.h>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>
#include <eagle_clientsocket.h>
#include <libconfig.h++>

#include <log.h>
#include "raft_global.h"
#include "raft_receive_work.h"
#include "raft_handle_work.h"
#include "raft_leader_work.h"
#include "raft_candidate_work.h"
#include "raft_follower_work.h"
#include "raft_send_work.h"

namespace RAFT
{

using namespace std;
using namespace eagle;
using namespace libconfig;

class Raft : public Thread
{
//构造与析构, 单例模式, 不允许从类外生成类的实例
protected:
	Raft();
   ~Raft();
   Raft(const Raft&);
   Raft& operator = (const Raft&);

public:
    //初始化函数
    static int Init(const char* ip);
	
	//类外获取Raft实例的接口
	static Raft* GetInstance();

	//读配置文件函数
	static int ParseConfigFile(struct RaftGlobal::RaftParameter& para, const char* config);

	//检测本机是否是中心节点
	static bool IsLeader() { return RaftHandleWork::Get_Charactor() == RaftGlobal::LEADER; }

protected:
    //线程的启动函数
    virtual void run();

	//报文解析函数
	static int Parse(string &buffer, string &o);

	//启动各工作线程函数
	int StartThread();

private:
	//单例实例指针
	static Raft* _instance;

    //本机ip
	static string _ip;

	//保存raft参数的具体对象
	static RaftGlobal::RaftParameter _raftpara;

	//全局服务器变量
	static EpollServerPtr g_server;
};

}

#endif //RAFT_H

// vim: ts=4 sw=4 nu
