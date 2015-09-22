/* author	: zhangqi
 * filename	: datatask_node.h
 * version	: 0.1
 */

#ifndef _DATATASK_NODE_H_
#define _DATATASK_NODE_H_

#include <boost/asio.hpp>

class redisworker;
class em_mongodb;
class KSM_ZK;

typedef struct _data_task_
{
	std::string key;
	std::string task;
}data_task;

class datatask_node
{
public:
	datatask_node(boost::asio::io_service& ioService,std::string iniFile);
	virtual ~datatask_node();

	virtual int readIni(std::string iniFile);
	virtual int init(std::string iniFile);
	virtual int run();
	virtual void parsepara(std::string redistask);

	virtual int writeResult(std::string value);
	virtual int doTask();
private:
	redisworker m_redis;
	em_mongodb  m_mongodb;
	KSM_ZK		m_zookeeper;
	boost::asio::io_service& m_ioService;
};	

#endif
