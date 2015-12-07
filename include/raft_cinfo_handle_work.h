#ifndef RAFT_CINFO_HANDLE_WORK_H
#define RAFT_CINFO_HANDLE_WORK_H
/**
 * @file    raft_cinfo_handle_work.h
 * @brief   client info�����߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-09-08
 */
#include <math.h>
#include <time.h>
#include <string>
#include <stdlib.h>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>
#include <eagle_simple_clientsocket.h>

#include <log.h>
#include <urcu/uatomic.h>
#include "raft_global.h"
#include "raft_lockfree_map.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftCInfoHandleWork : public Thread
{
public:
	//����������
	 RaftCInfoHandleWork();
	~RaftCInfoHandleWork();

	//��ʼ������
	static void Init(string ip, RaftGlobal::RaftParameter &para, int server_port);

	//��Ԫ��,�õ������еĿͻ����б�map
	friend class RaftCInfoSendWork;
	friend class RaftNotifyClientWork;

protected:
	//�̵߳�����ջ֡
	virtual void run();

public:
	//����ָ��
	static RaftCInfoHandleWork *s_raft_cinfo_handle_work_arr[RaftGlobal::skRaftCInfoHandleWorkNum];

private:
	//����ip
	static string _ip;
	//raft���ò�������
	static RaftGlobal::RaftParameter _raftparameter;
	//������˿�
	static int _server_port;
	//�л���, �����л�Follower�б�����ʱ
	static int s_curr_switch_no;
	//Follower����
	static int s_follower_num[2];
	//ip�б�
	static string s_ip_arr[2][RaftGlobal::skFollowerMaxCount];
	//port�б�
	static int s_port_arr[2][RaftGlobal::skFollowerMaxCount];
	//Ψһ���id�б�
	static int s_id_arr[2][RaftGlobal::skFollowerMaxCount];
	
	//��ȡ��ǰ�л���
	static int GetCurrSwitchNo() { return uatomic_read(&s_curr_switch_no); }
	//��ȡ�´ε��л���
	static int GetNextSwitchNo() { return !s_curr_switch_no; }
	//�л���ǰ���л���
	static void Switch() { uatomic_set(&s_curr_switch_no, !s_curr_switch_no); }
	//����raft�����ļ�
	static void SwitchAll(RaftGlobal::RaftParameter _raftparameter);

private:
	//follower��ͨ��Socket
	static TCPSimpleClient _sock_arr[2][RaftGlobal::skFollowerMaxCount];
	//ά���Ŀͻ����б�
	static mthread_map<std::string, uint64_t> _client_list;
	//ʱ���
	struct timeval _tv;
};

}

#endif //RAFT_CINFO_HANDLE_WORK_H

// vim: ts=4 sw=4 nu

