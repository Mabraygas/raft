#include "raft.h"

namespace RAFT
{

string Raft::_ip;
RaftGlobal::RaftParameter Raft::_raftpara;
EpollServerPtr Raft::g_server;
	
Raft::Raft()
{ }

Raft::~Raft()
{ }

int Raft::Init(const char* ip)
{
	//本机ip
	_ip = string(ip);
	
	//获取参数
	int iRet = ParseConfigFile(_raftpara, RaftGlobal::skRaftConfigFile);
	if(0 != iRet) {
		INFO("Raft: ParseConfigFile Error!");
	}else {
		INFO("Raft: ParseConfigFile Succ!");
	}
	
	return iRet;
}

int Raft::ParseConfigFile(RaftGlobal::RaftParameter& para, const char* config_path) {
	//libconfig配置对象
	Config cfg;

	try {
		//加载配置文件
		cfg.readFile(config_path);
	}catch(const FileIOException &fioex) {
		fprintf(stderr, "不存在raft配置文件: %s\n", RaftGlobal::skRaftConfigFile);
		return -1;
	}catch(const ParseException &pex) {
		cerr << "raft配置文件第" << pex.getLine() << "行解析错误: " << pex.getError() << endl;
		return -1;
	}

	//字段: Follower_N
	if(!cfg.lookupValue("FOLLOWER_N", para.follower_N)) {
		fprintf(stderr, "Follower的总数(FOLLOWER_N)字段不存在!\n");
		return -1;
	}
	if(para.follower_N > RaftGlobal::skFollowerMaxCount) {
		fprintf(stderr, "Follower的总数(%d)不能超过Max(%d)!\n", para.follower_N, RaftGlobal::skFollowerMaxCount);
		return -1;
	}
	if(para.follower_N <= 0) {
		fprintf(stderr, "Follower的总数(%d)不能为0!\n", para.follower_N);
		return -1;
	}

	//字段: follower_list
	try {
		const Setting &follower_list = cfg.lookup("follower_list");
		if(follower_list.getLength() != para.follower_N) {
			fprintf(stderr, "Follower列表项数(%d)与总数(%d)不符!\n", follower_list.getLength(), para.follower_N);
			return -1;
		}
		//依次读取每个follower的信息
		for(int i = 0; i < para.follower_N; i ++ ) {
			const Setting &follower_setting = follower_list[i];
			if(!(follower_setting.lookupValue("ip", para.follower_list[i].ip)
			  && follower_setting.lookupValue("port", para.follower_list[i].port)
			  && follower_setting.lookupValue("id", para.follower_list[i].id))) {
				fprintf(stderr, "获取第%d个Follower信息失败!\n", i);
				return -1;
			}
			//检测参数范围
			if(para.follower_list[i].port < 1024 || para.follower_list[i].port > 65535) {
				fprintf(stderr, "第%d个Follower的Port(%d)不在合理范围(1024-65535)内!\n", i, para.follower_list[i].port);
				return -1;
			}
			if(para.follower_list[i].id < 0 || para.follower_list[i].id >= RaftGlobal::skFollowerMaxCount) {
				fprintf(stderr, "第%d个Follower的id(%d)不在合法范围(0-%d)内!\n", i, para.follower_list[i].id, RaftGlobal::skFollowerMaxCount);
				return -1;
			}
		}
	}catch(const SettingNotFoundException &nfex) {
		fprintf(stderr, "Follower信息(follower_list)字段不存在!\n");
		return -1;
	}

	//判断id的完备性, 并根据id大小对Follower重排序
	
	//完备性判断
	uint64_t bit_ruler[64];
	bit_ruler[0] = 0x8000000000000000ul;
	for(int i = 1; i < 64; i++ ) {
		bit_ruler[i] = bit_ruler[i - 1] >> 1;
	}
	//靶
	uint64_t target = 0ul;
	for(int i = 0; i < para.follower_N; i ++ ) {
		target |= bit_ruler[para.follower_list[i].id];
	}
	for(int i = 0; i < para.follower_N; i ++ ) {
		if(bit_ruler[i] != (target & bit_ruler[i])) {
			fprintf(stderr, "Follower ID不具有完备性(0 - N-1)!\n");
			return -1;
		}
	}

	//重排序
	string ip_arr[RaftGlobal::skFollowerMaxCount];
	int    port_arr[RaftGlobal::skFollowerMaxCount];
	for(int i = 0; i < para.follower_N; i ++ ) {
		ip_arr[para.follower_list[i].id]   = para.follower_list[i].ip;
		port_arr[para.follower_list[i].id] = para.follower_list[i].port;
	}
	for(int i = 0 ;i < para.follower_N; i ++ ) {
		para.follower_list[i].ip   = ip_arr[i];
		para.follower_list[i].port = port_arr[i];
	}

	//字段: Forced_Leader
	if(!cfg.lookupValue("Forced_Leader", para.forced_leader)) {
		fprintf(stderr, "强制L1(Forced_Leader)字段不存在!\n");
		return -1;
	}

	//字段: HeartBeat_Interval
	if(!cfg.lookupValue("HeartBeat_Interval", para.heartbeat_interval)) {
		fprintf(stderr, "Follower心跳时间间隔(HeartBeat_Interval)字段不存在!\n");
		return -1;
	}

	//字段: Client_Port
	if(!cfg.lookupValue("Client_Port", para.client_port)) {
		fprintf(stderr, "通知客户端的端口号(Client_Port)字段不存在!\n");
		return -1;
	}

	//打印
	INFO("Raft: Raft Config:");
	INFO("Raft: FOLLOWER_N: " << para.follower_N);
	for(int i = 0; i < para.follower_N; i ++ ) {
		INFO("Raft: Follower" << i << ": IP " << para.follower_list[i].ip << " Port " << para.follower_list[i].port << " ID " << para.follower_list[i].id);
	}
	INFO("Raft: Forced_Leader: " << para.forced_leader);
	INFO("Raft: HeartBeat_Interval: " << para.heartbeat_interval);
	INFO("Raft: Client_Port: " << para.client_port);
	
	//解析成功
	return 0;
}

void Raft::run() {

	//服务器
	g_server = new EpollServer();
	
	//适配器
	BindAdapterPtr adapter = new BindAdapter(g_server);

	int port = -1;
	//根据配置文件, 查找本机的raft心跳端口
	for(int i = 0; i < _raftpara.follower_N; i ++ ) {
		if(_raftpara.follower_list[i].ip == _ip) {
			port = _raftpara.follower_list[i].port;
		}
	}
	//如果在配置文件中没找到本机
	if(-1 == port) {
		//保存, 退出
		ERROR("Not find local: " << _ip << " in Raft config file: " << RaftGlobal::skRaftConfigFile);
		return;
	}

	//IP, 端口, 超时(300s), TCP
	Endpoint local(_ip, port, 300000, true);

	adapter->setEndpoint(local);

	//报文协议
	adapter->setProtocol(Parse);

	//创建处理线程组, 并将适配器加入服务器
	g_server->CreateWorkGroup<RaftReceiveWork>("RaftReceiveWork", RaftGlobal::skRaftRecvWorkNum, adapter);

	//启动各工作线程
	if(0 != StartThread()) {
		ERROR("Start Working Thread Error! Stop Raft.");
		return;
	}
	
	//运行服务: 不断接收请求, 循环处理
	g_server->EnterMainLoop();

}

int Raft::Parse(string &buffer, string &o) {

	//判断包体长度: key(8) + ver(1) + client_list(1) + type(4) + F/C/L(1) + step(4)
	if(buffer.length() < 19) {
		return PACKET_LESS;
	}

	//判断Key、版本
	if(*(uint64_t *)RaftGlobal::skRaftHeartBeatKey != *(uint64_t *)buffer.c_str()
	   || RaftGlobal::skRaftHeartBeatVer != *(uint8_t *)(buffer.c_str() + 8)) {
		buffer = buffer.substr(9, buffer.length() - 9);
		return PACKET_ERR;
	}

	//心跳或客户端列表标志
	uint8_t client_list = *(uint8_t *)(buffer.c_str() + 9);
	
	if(!client_list) { //是心跳
		//心跳种类
		int type = *(int *)(buffer.c_str() + 10);
		if(type < 0 || type > 3) {
			buffer = buffer.substr(14, buffer.length() - 14);
			return PACKET_ERR;
		}

		o = buffer.substr(9, 10);
		//在buffer中去掉该数据包
		buffer = buffer.substr(19, buffer.length() - 19);

		return PACKET_FULL;
	}else { //是客户端列表
		
		//ip地址长度
		int ip_length = *(int *)(buffer.c_str() + 10);

		//判断包体长度: key(8) + ver(1) + client_list(1) + ip_length(4) + ip(ip_length)
		if(buffer.length() < (size_t)(14 + ip_length)) {
			return PACKET_LESS;
		}

		if(ip_length < 0 || ip_length > 15) {
			buffer = buffer.substr(14, buffer.length() - 14);
			return PACKET_ERR;
		}

		o = buffer.substr(9, 5 + ip_length);
		//在buffer中去掉该数据包
		buffer = buffer.substr(14 + ip_length, buffer.length() - 14 - ip_length);

		return PACKET_FULL;
	}
}

int Raft::StartThread() {
	
	try {
		
		//初始化并启动Raft心跳解析线程
		RaftHandleWork::Init(_ip, _raftpara);
		for(int gid = 0; gid < RaftGlobal::skRaftHandleWorkNum; gid ++ ) {
			RaftHandleWork::s_raft_handle_work_arr[gid] = new RaftHandleWork();
			(RaftHandleWork::s_raft_handle_work_arr[gid])->start();
		}

		//启动Raft角色工作线程(3种: L/C/F)
		for(int gid = 0; gid < RaftGlobal::skRaftLeaderWorkNum; gid ++ ) {
			RaftLeaderWork::s_raft_leader_work_arr[gid] = new RaftLeaderWork();
			(RaftLeaderWork::s_raft_leader_work_arr[gid])->start();
		}
		for(int gid = 0; gid < RaftGlobal::skRaftCandidateWorkNum; gid ++ ) {
			RaftCandidateWork::s_raft_candidate_work_arr[gid] = new RaftCandidateWork();
			(RaftCandidateWork::s_raft_candidate_work_arr[gid])->start();
		}
		for(int gid = 0; gid < RaftGlobal::skRaftFollowerWorkNum; gid ++ ) {
			RaftFollowerWork::s_raft_follower_work_arr[gid] = new RaftFollowerWork();
			(RaftFollowerWork::s_raft_follower_work_arr[gid])->start();
		}
		
		//启动Raft心跳发送线程
		RaftSendWork::s_server = g_server;
		for(int gid = 0; gid < RaftGlobal::skRaftSendWorkNum; gid ++ ) {
			RaftSendWork::s_raft_send_work_arr[gid] = new RaftSendWork();
			(RaftSendWork::s_raft_send_work_arr[gid])->start();
		}
		
		//启动raft通知Client线程
		RaftNotifyClientWork::s_server = g_server;
		for(int gid = 0; gid < RaftGlobal::skRaftNotifyClientWorkNum; gid ++ ) {
			RaftNotifyClientWork::s_raft_notify_client_work_arr[gid] = new RaftNotifyClientWork();
			(RaftNotifyClientWork::s_raft_notify_client_work_arr[gid])->start();
		}
		
	}catch(EagleException &ex) {
		fprintf(stderr, "Start Thread Error!\n");
		return -1;
	}
	//启动成功
	return 0;
}

};

// vim: ts=4 sw=4 nu
