#ifndef RAFT_H
#define RAFT_H

/**
 * @file 	raft.h
 * @brief   ��ѡ��ģ��
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
#include "raft_notify_client_work.h"

namespace RAFT
{

using namespace std;
using namespace eagle;
using namespace libconfig;

class Raft : public Thread
{
//����������
public:
	Raft();
   ~Raft();

public:
    //��ʼ������
    static int Init(const char* ip);

	//�������ļ�����
	static int ParseConfigFile(struct RaftGlobal::RaftParameter& para, const char* config);

protected:
    //�̵߳���������
    virtual void run();

	//���Ľ�������
	static int Parse(string &buffer, string &o);

	//�����������̺߳���
	int StartThread();

private:
    //����ip
	static string _ip;

	//����raft�����ľ������
	static RaftGlobal::RaftParameter _raftpara;

	//ȫ�ַ���������
	static EpollServerPtr g_server;
};

}

#endif //RAFT_H

// vim: ts=4 sw=4 nu
