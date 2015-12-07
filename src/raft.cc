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
	//����ip
	_ip = string(ip);
	
	//��ȡ����
	int iRet = ParseConfigFile(_raftpara, RaftGlobal::skRaftConfigFile);
	if(0 != iRet) {
		INFO("Raft: ParseConfigFile Error!");
	}else {
		INFO("Raft: ParseConfigFile Succ!");
	}
	
	return iRet;
}

int Raft::ParseConfigFile(RaftGlobal::RaftParameter& para, const char* config_path) {
	//libconfig���ö���
	Config cfg;

	try {
		//���������ļ�
		cfg.readFile(config_path);
	}catch(const FileIOException &fioex) {
		fprintf(stderr, "������raft�����ļ�: %s\n", RaftGlobal::skRaftConfigFile);
		return -1;
	}catch(const ParseException &pex) {
		cerr << "raft�����ļ���" << pex.getLine() << "�н�������: " << pex.getError() << endl;
		return -1;
	}

	//�ֶ�: Follower_N
	if(!cfg.lookupValue("FOLLOWER_N", para.follower_N)) {
		fprintf(stderr, "Follower������(FOLLOWER_N)�ֶβ�����!\n");
		return -1;
	}
	if(para.follower_N > RaftGlobal::skFollowerMaxCount) {
		fprintf(stderr, "Follower������(%d)���ܳ���Max(%d)!\n", para.follower_N, RaftGlobal::skFollowerMaxCount);
		return -1;
	}
	if(para.follower_N <= 0) {
		fprintf(stderr, "Follower������(%d)����Ϊ0!\n", para.follower_N);
		return -1;
	}

	//�ֶ�: follower_list
	try {
		const Setting &follower_list = cfg.lookup("follower_list");
		if(follower_list.getLength() != para.follower_N) {
			fprintf(stderr, "Follower�б�����(%d)������(%d)����!\n", follower_list.getLength(), para.follower_N);
			return -1;
		}
		//���ζ�ȡÿ��follower����Ϣ
		for(int i = 0; i < para.follower_N; i ++ ) {
			const Setting &follower_setting = follower_list[i];
			if(!(follower_setting.lookupValue("ip", para.follower_list[i].ip)
			  && follower_setting.lookupValue("port", para.follower_list[i].port)
			  && follower_setting.lookupValue("id", para.follower_list[i].id))) {
				fprintf(stderr, "��ȡ��%d��Follower��Ϣʧ��!\n", i);
				return -1;
			}
			//��������Χ
			if(para.follower_list[i].port < 1024 || para.follower_list[i].port > 65535) {
				fprintf(stderr, "��%d��Follower��Port(%d)���ں���Χ(1024-65535)��!\n", i, para.follower_list[i].port);
				return -1;
			}
			if(para.follower_list[i].id < 0 || para.follower_list[i].id >= RaftGlobal::skFollowerMaxCount) {
				fprintf(stderr, "��%d��Follower��id(%d)���ںϷ���Χ(0-%d)��!\n", i, para.follower_list[i].id, RaftGlobal::skFollowerMaxCount);
				return -1;
			}
		}
	}catch(const SettingNotFoundException &nfex) {
		fprintf(stderr, "Follower��Ϣ(follower_list)�ֶβ�����!\n");
		return -1;
	}

	//�ж�id���걸��, ������id��С��Follower������
	
	//�걸���ж�
	uint64_t bit_ruler[64];
	bit_ruler[0] = 0x8000000000000000ul;
	for(int i = 1; i < 64; i++ ) {
		bit_ruler[i] = bit_ruler[i - 1] >> 1;
	}
	//��
	uint64_t target = 0ul;
	for(int i = 0; i < para.follower_N; i ++ ) {
		target |= bit_ruler[para.follower_list[i].id];
	}
	for(int i = 0; i < para.follower_N; i ++ ) {
		if(bit_ruler[i] != (target & bit_ruler[i])) {
			fprintf(stderr, "Follower ID�������걸��(0 - N-1)!\n");
			return -1;
		}
	}

	//������
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

	//�ֶ�: Forced_Leader
	if(!cfg.lookupValue("Forced_Leader", para.forced_leader)) {
		fprintf(stderr, "ǿ��L1(Forced_Leader)�ֶβ�����!\n");
		return -1;
	}

	//�ֶ�: HeartBeat_Interval
	if(!cfg.lookupValue("HeartBeat_Interval", para.heartbeat_interval)) {
		fprintf(stderr, "Follower����ʱ����(HeartBeat_Interval)�ֶβ�����!\n");
		return -1;
	}

	//�ֶ�: Client_Port
	if(!cfg.lookupValue("Client_Port", para.client_port)) {
		fprintf(stderr, "֪ͨ�ͻ��˵Ķ˿ں�(Client_Port)�ֶβ�����!\n");
		return -1;
	}

	//��ӡ
	INFO("Raft: Raft Config:");
	INFO("Raft: FOLLOWER_N: " << para.follower_N);
	for(int i = 0; i < para.follower_N; i ++ ) {
		INFO("Raft: Follower" << i << ": IP " << para.follower_list[i].ip << " Port " << para.follower_list[i].port << " ID " << para.follower_list[i].id);
	}
	INFO("Raft: Forced_Leader: " << para.forced_leader);
	INFO("Raft: HeartBeat_Interval: " << para.heartbeat_interval);
	INFO("Raft: Client_Port: " << para.client_port);
	
	//�����ɹ�
	return 0;
}

void Raft::run() {

	//������
	g_server = new EpollServer();
	
	//������
	BindAdapterPtr adapter = new BindAdapter(g_server);

	int port = -1;
	//���������ļ�, ���ұ�����raft�����˿�
	for(int i = 0; i < _raftpara.follower_N; i ++ ) {
		if(_raftpara.follower_list[i].ip == _ip) {
			port = _raftpara.follower_list[i].port;
		}
	}
	//����������ļ���û�ҵ�����
	if(-1 == port) {
		//����, �˳�
		ERROR("Not find local: " << _ip << " in Raft config file: " << RaftGlobal::skRaftConfigFile);
		return;
	}

	//IP, �˿�, ��ʱ(300s), TCP
	Endpoint local(_ip, port, 300000, true);

	adapter->setEndpoint(local);

	//����Э��
	adapter->setProtocol(Parse);

	//���������߳���, �������������������
	g_server->CreateWorkGroup<RaftReceiveWork>("RaftReceiveWork", RaftGlobal::skRaftRecvWorkNum, adapter);

	//�����������߳�
	if(0 != StartThread()) {
		ERROR("Start Working Thread Error! Stop Raft.");
		return;
	}
	
	//���з���: ���Ͻ�������, ѭ������
	g_server->EnterMainLoop();

}

int Raft::Parse(string &buffer, string &o) {

	//�жϰ��峤��: key(8) + ver(1) + client_list(1) + type(4) + F/C/L(1) + step(4)
	if(buffer.length() < 19) {
		return PACKET_LESS;
	}

	//�ж�Key���汾
	if(*(uint64_t *)RaftGlobal::skRaftHeartBeatKey != *(uint64_t *)buffer.c_str()
	   || RaftGlobal::skRaftHeartBeatVer != *(uint8_t *)(buffer.c_str() + 8)) {
		buffer = buffer.substr(9, buffer.length() - 9);
		return PACKET_ERR;
	}

	//������ͻ����б��־
	uint8_t client_list = *(uint8_t *)(buffer.c_str() + 9);
	
	if(!client_list) { //������
		//��������
		int type = *(int *)(buffer.c_str() + 10);
		if(type < 0 || type > 3) {
			buffer = buffer.substr(14, buffer.length() - 14);
			return PACKET_ERR;
		}

		o = buffer.substr(9, 10);
		//��buffer��ȥ�������ݰ�
		buffer = buffer.substr(19, buffer.length() - 19);

		return PACKET_FULL;
	}else { //�ǿͻ����б�
		
		//ip��ַ����
		int ip_length = *(int *)(buffer.c_str() + 10);

		//�жϰ��峤��: key(8) + ver(1) + client_list(1) + ip_length(4) + ip(ip_length)
		if(buffer.length() < (size_t)(14 + ip_length)) {
			return PACKET_LESS;
		}

		if(ip_length < 0 || ip_length > 15) {
			buffer = buffer.substr(14, buffer.length() - 14);
			return PACKET_ERR;
		}

		o = buffer.substr(9, 5 + ip_length);
		//��buffer��ȥ�������ݰ�
		buffer = buffer.substr(14 + ip_length, buffer.length() - 14 - ip_length);

		return PACKET_FULL;
	}
}

int Raft::StartThread() {
	
	try {
		
		//��ʼ��������Raft���������߳�
		RaftHandleWork::Init(_ip, _raftpara);
		for(int gid = 0; gid < RaftGlobal::skRaftHandleWorkNum; gid ++ ) {
			RaftHandleWork::s_raft_handle_work_arr[gid] = new RaftHandleWork();
			(RaftHandleWork::s_raft_handle_work_arr[gid])->start();
		}

		//����Raft��ɫ�����߳�(3��: L/C/F)
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
		
		//����Raft���������߳�
		RaftSendWork::s_server = g_server;
		for(int gid = 0; gid < RaftGlobal::skRaftSendWorkNum; gid ++ ) {
			RaftSendWork::s_raft_send_work_arr[gid] = new RaftSendWork();
			(RaftSendWork::s_raft_send_work_arr[gid])->start();
		}
		
		//����raft֪ͨClient�߳�
		RaftNotifyClientWork::s_server = g_server;
		for(int gid = 0; gid < RaftGlobal::skRaftNotifyClientWorkNum; gid ++ ) {
			RaftNotifyClientWork::s_raft_notify_client_work_arr[gid] = new RaftNotifyClientWork();
			(RaftNotifyClientWork::s_raft_notify_client_work_arr[gid])->start();
		}
		
	}catch(EagleException &ex) {
		fprintf(stderr, "Start Thread Error!\n");
		return -1;
	}
	//�����ɹ�
	return 0;
}

};

// vim: ts=4 sw=4 nu
