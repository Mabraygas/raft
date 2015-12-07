#ifndef RAFT_SEND_WORK_H
#define RAFT_SEND_WORK_H
/**
 * @file    raft_send_work.h
 * @brief   raft�����߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <string>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>

#include <log.h>
#include "raft_global.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftSendWork : public Thread
{
public:
	//����������
	 RaftSendWork();
	~RaftSendWork();

protected:
	//�̵߳�����ջ֡
	virtual void run();

public:
	static RaftSendWork *s_raft_send_work_arr[RaftGlobal::skRaftSendWorkNum];
	
private:
	/**
	 * @brief ����ظ�����
	 *
	 * @param [item]: ���͵����� 
	 * @param [resp]: ���汨�ĵĻ�����
	 *
	 * @return : ���ĳ���
	 */
	size_t ConstructResponse(const RaftGlobal::RaftSendHeartBeatItem &item, char *resp);

public:
	//������ָ��
	static EpollServerPtr s_server;
};

}

#endif //RAFT_SEND_WORK_H

// vim: ts=4 sw=4 nu

