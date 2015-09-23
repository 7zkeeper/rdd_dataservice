/* author	: zhangqi
 * filename	: datatask_node.h
 * version	: 0.1
 */

#ifndef _DATATASK_NODE_H_
#define _DATATASK_NODE_H_

#include <boost/asio.hpp>

#include "redis/redisworker.h"
#include "zookeeper/ksm_zk.h"
#include "mongodb/em_mongodb.h"

typedef struct _data_task_
{
	std::string key;
	std::string task;
}data_task;

class datatask_node
{
public:
	datatask_node(boost::asio::io_service& ioService,RedisAsyncClient& rd,std::string iniFile);
	virtual ~datatask_node();

	virtual int readIni(std::string iniFile);
	virtual int init(std::string iniFile);
	virtual int run();
	virtual void parsepara(std::string redistask);

	virtual int writeResult(std::string value);
	virtual void  doTask(const RedisValue& value);
private:
	redisworker m_redis;
	em_mongodb  m_mongodb;
	KSM_ZK		m_zookeeper;
	boost::asio::io_service& m_ioService;
};	

#endif