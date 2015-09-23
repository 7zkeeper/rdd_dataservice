/* author	: zhangqi
 * filename	: datatask_node.cpp
 * version	: 0.1
 */


#include "inifile/inifile.h"
#include "datatask_node.h"

datatask_node::datatask_node(boost::asio::io_service& ioService,
				RedisAsyncClient& rd,std::string iniFile)
	: m_ioService(ioService),m_redis(ioService,rd)
{
	init(iniFile);
}

datatask_node:: ~datatask_node()
{

}

int datatask_node::readIni(std::string iniFile)
{
	int ret = 0;
	m_redis.readIni(iniFile);
	m_mongodb.readIni(iniFile);
	m_zookeeper.readIni(iniFile);
	return ret;
}

int datatask_node::init(std::string iniFile)
{
	int ret = 0;
	readIni(iniFile);
	m_redis.init(boost::bind(&datatask_node::doTask,this,_1));
	m_zookeeper.init(iniFile);
	m_mongodb.init(iniFile);
	return ret;
}

int datatask_node::run()
{
	return 1;
}

int datatask_node::writeResult(std::string value)
{
	m_redis.getRedis().command("LPUSH",m_redis.getRetTitle(),value,boost::bind(&redisworker::onLpush,&m_redis,_1));
	return 1;
}

void datatask_node::doTask(const RedisValue& value)
{
//	m_taskdb(value);
	parsepara(value.toString());
	return 1;
}

void datatask_node::parsepara(std::string redistask)
{
	std::string dbcoll,value;
	mongo::Query cond;
	BSONObj valobj;
	
	int type = 1;
	if(type == 1)
	{
		m_mongodb.query(dbcoll,cond,value);
	}
	else if( type == 2)
	{
		m_mongodb.setvalue(dbcoll,cond,valobj,true);
		value = "OK";
	}
	writeResult(value);
}

