#ifndef RAFT_NOTIFY_CLIENT_WORK_H
#define RAFT_NOTIFY_CLIENT_WORK_H
/**
 * @file    raft_notify_client_work.h
 * @brief   raft֪ͨ�ͻ����߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <map>
#include <string>
#include <eagle_thread.h>
#include <eagle_epoll_server.h>

#include <log.h>
#include "raft_global.h"
#include "raft_cinfo_handle_work.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

class RaftNotifyClientWork : public Thread
{
public:
	//����������
	 RaftNotifyClientWork();
	~RaftNotifyClientWork();

	//��ʼ������
	void Init(int notify_port);

protected:
	//�̵߳�����ջ֡
	virtual void run();

public:
	static RaftNotifyClientWork *s_raft_notify_client_work_arr[RaftGlobal::skRaftNotifyClientWorkNum];
	//����Leader֪ͨ����Client���ܺ���
	void Notify();
	
private:
	/**
	 * @brief ����ظ�����
	 *
	 * @param [resp]: ���汨�ĵĻ�����
	 *
	 * @return : ���ĳ���
	 */
	size_t ConstructNotify(char *resp);

public:
	//������ָ��
	static EpollServerPtr s_server;

private:
	//��ѯ���Ŀͻ����б�
	map<std::string, uint64_t> _client_list;
	//�ͻ��˽���֪ͨ�˿�
	static int _notify_port;
};

}

#endif //RAFT_NOTIFY_CLIENT_WORK_H

// vim: ts=4 sw=4 nu

