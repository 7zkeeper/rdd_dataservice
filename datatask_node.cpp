/* author	: zhangqi
 * filename	: datatask_node.cpp
 * version	: 0.1
 */


#include "src/redis/redisworker.h"
#include "src/inifiel/inifile.h"
#include "src/zookeeper/ksm_zk.h"
#include "src/mongodb/em_mongodb.h"
#include "datatask_node.h"

datatask_node::datatask_node(std::string iniFile)
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
	m_mongo.readIni(iniFile);
	m_zookeeper.read.Ini(iniFile);
	return ret;
}

int datatask_node::init(std::string iniFile)
{
	int ret = 0;
	readIni(iniFile);
	m_redis.init(boost::bind(&datatask_node::doTask,this,_1));
	m_zookeeper.init();
	m_mongo.init();
	return ret;
}

int datatask_node::run()
{
	return 1;
}

int datatask_node::writeResult(std::string value)
{
	m_redis.command("LPUSH",m_redis.getRetTitle(),value,boost::bind(&redisworker::onLpush,&m_redis,_1));
	return 1;
}

int datatask_node::doTask(const RedisValue& value)
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
		m_mongo.query(dbcoll,cond,value);
	}
	else if( type == 2)
	{
		m_mongo.setvalue(dbcoll,cond,valobj,true);
		value = "OK";
	}
	writeResult(value);
}

