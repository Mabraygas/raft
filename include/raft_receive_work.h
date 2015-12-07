#ifndef RAFT_RECEIVE_WORK_H
#define RAFT_RECEIVE_WORK_H
/**
 * @file    raft_receive_work.h
 * @brief   raft接收线程
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
 * raft接收线程
 */
class RaftReceiveWork : public Work
{
public :
    /**
     * @brief 默认构造函数
     */
    RaftReceiveWork() {}

    /**
     * @brief 析构函数
     */
    ~RaftReceiveWork() {}

protected:
    /** 
     * @brief 线程启动前的初始化.
     */
    virtual void initialize() {}

    /**
     * @brief 线程启动时进入循环处理前的回调函数.
     */
    virtual void startHandle() {}

    /** 
     * @brief 逻辑处理函数
     *
     * @param [recv] : Adapter接收队列中的元素
     */
    virtual void work(const RecvData &recv);

	/**
     * @brief 逻辑处理完毕(也就是跳出循环处理后)时的回调函数
     * 等待Broker线程退出.
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
