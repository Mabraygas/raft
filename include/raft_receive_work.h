#ifndef RAFT_RECEIVE_WORK_H
#define RAFT_RECEIVE_WORK_H
/**
 * @file    raft_receive_work.h
 * @brief   raft�����߳�
 * @author  maguangxu
 * @version 1.0
 * @date    2015-08-27
 */
#include <string>
#include <log.h>
#include <eagle_epoll_server.h>

#include "raft_global.h"

namespace RAFT
{

using namespace std;
using namespace eagle;

/**
 * raft�����߳�
 */
class RaftReceiveWork : public Work
{
public :
    /**
     * @brief Ĭ�Ϲ��캯��
     */
    RaftReceiveWork() {}

    /**
     * @brief ��������
     */
    ~RaftReceiveWork() {}

protected:
    /** 
     * @brief �߳�����ǰ�ĳ�ʼ��.
     */
    virtual void initialize() {}

    /**
     * @brief �߳�����ʱ����ѭ������ǰ�Ļص�����.
     */
    virtual void startHandle() {}

    /** 
     * @brief �߼�������
     *
     * @param [recv] : Adapter���ն����е�Ԫ��
     */
    virtual void work(const RecvData &recv);

	/**
     * @brief �߼��������(Ҳ��������ѭ�������)ʱ�Ļص�����
     * �ȴ�Broker�߳��˳�.
     */
    virtual void stopHandle() {}

    virtual void WorkOverload(const RecvData &recv)
    {
		WARN("overload");
    }

    virtual void WorkTimeout(const RecvData &recv)
    {
		WARN("timeout");
    }
};

}

#endif //RAFT_RECEIVE_WORK_H

// vim: ts=4 sw=4 nu
