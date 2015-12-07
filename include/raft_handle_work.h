#ifndef RAFT_HANDLE_WORK_H
#define RAFT_HANDLE_WORK_H
/**
 * @file    raft_handle_work.h
 * @brief   raft�����߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
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

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftHandleWork : public Thread
{
public:
	//����������
	 RaftHandleWork();
	~RaftHandleWork();

	//��ʼ������
	static void Init(string ip, RaftGlobal::RaftParameter &para);

	//������Ԫ��
	friend class RaftLeaderWork;
	friend class RaftCandidateWork;
	friend class RaftFollowerWork;
	
protected:
	//�̵߳�����ջ֡
	virtual void run();

public:
	//����ָ��
	static RaftHandleWork *s_raft_handle_work_arr[RaftGlobal::skRaftHandleWorkNum];

private:
	//����ip
	static string _ip;
	//raft���ò�������
	static RaftGlobal::RaftParameter _raftparameter;
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
	//�����Ľ�ɫ(F/C/L)
	static char _charactor;
	//Ŀǰ�����Ľ׶�
	static int  _step;
	//�Ѿ������Ľ׶κ�CandidateͶ��Ʊ��
	static int  _voted;
	//�����ȴ�Leader������ʱ��(ʱ�������������ΪCandidate)
	static int  _timewait;

public:
	//��д������Ϣ��һ�麯��
	static char Get_Charactor() 		{ return uatomic_read(&_charactor); }
	static void Set_Charactor(char des) { uatomic_set(&_charactor, des); }
	static int  Get_Step() 				{ return uatomic_read(&_step); }
	static void Set_Step(int des) 		{ uatomic_set(&_step, des); }
	static int  Get_Voted() 			{ return uatomic_read(&_voted); }
	static void Set_Voted(int des)		{ uatomic_set(&_voted, des); }
	static int  Get_TimeWait() 			{ return uatomic_read(&_timewait); }
	static void Set_TimeWait(int des) 	{ uatomic_set(&_timewait, des); }


private:
	static TCPSimpleClient _sock_arr[2][RaftGlobal::skFollowerMaxCount];
	
};

}

#endif //RAFT_HANDLE_WORK_H

// vim: ts=4 sw=4 nu

