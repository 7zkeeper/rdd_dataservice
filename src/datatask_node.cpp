/* author	: zhangqi
 * filename	: datatask_node.cpp
 * version	: 0.1
 */


#include "inifile/inifile.h"
#include "datatask_node.h"

//using namespace mongo;

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

void datatask_node::updateUserStockCfg(std::string dbcoll,std::string uid,condition value)
{
	mongo::BSONObjBuilder c_bob,r_bob;
	mongo::BSONArrayBuilder sub;
	c_bob.append("UID",uid);
	c_bob.append("stocks.stockcode",value.valueary[0].c_str());
	
	std::string strret;
	bool c_res = true;
	m_mongodb.query(dbcoll,c_bob.obj(),strret);
	if(strret.size() < 10)
		c_res = false;
	else 
		c_res = true;
	
	int index = 0;
	mongo::BSONObjBuilder fieldbuild,stockbuild,finalbuild,cc_bob;
	
	cc_bob.append("UID",uid);		
	std::string retuid,retstockcode;
	int bulltin,run;
	float max_price=0.0f,min_price=0.0f;
	long long int increment=0;
	
	for(;index<value.keyary.size();index++)
	{
		char k[1024] = {0};
		if(!c_res)
			sprintf(k,"%s",value.keyary[index].c_str());
		else
			sprintf(k,"stocks.$.%s",value.keyary[index].c_str());
		
		std::string key = value.keyary[index];
		if(key == "uid")
		{
			retuid = value.valueary[index];
			if(!c_res)
				stockbuild.append(value.keyary[index].c_str(),retuid.c_str());
		}
		else if(key == "stockcode")
		{
			retstockcode = value.valueary[index];
			stockbuild.append(k,retstockcode.c_str());
		}
		else if(key == "max_price")
		{
			sscanf(value.valueary[index].c_str(),"%f",&max_price);
			if(max_price != 0.0f)
				stockbuild.append(k,max_price);
		}
		else if(key == "min_price")
		{
			sscanf(value.valueary[index].c_str(),"%f",&min_price);
			if(min_price != 0.0f)
				stockbuild.append(k,min_price);
		}
		else if(key == "bulltin")
		{
			sscanf(value.valueary[index].c_str(),"%d",&bulltin);
			stockbuild.append(k,bulltin);
		}
		else if(key == "run")
		{
			sscanf(value.valueary[index].c_str(),"%d",&run);
			stockbuild.append(k,run);
		}
		else if(key == "incrementId")
		{
			sscanf(value.valueary[index].c_str(),"%ld",&increment);
			stockbuild.append(k,increment);
		}
	}	
	mongo::Query query;
	if(!c_res)
	{
		fieldbuild.append("stocks",stockbuild.obj());
		finalbuild.append("$push",fieldbuild.obj());
		query = cc_bob.obj();
	}	
	else
	{
		cc_bob.append("stocks.stockcode",value.valueary[0].c_str());
		finalbuild.append("$set",stockbuild.obj());
		query = cc_bob.obj();
	}
	m_mongodb.setvalue(dbcoll.c_str(),query,finalbuild.obj(),true);
}

int datatask_node::setArrayInfo(std::string dbcoll,std::string kv,std::string mkey,std::string arrkey,condition value)
{
	std::string subkey;
	int pos = arrkey.find(".");
	if(pos > 0)
		subkey = arrkey.substr(pos+1,arrkey.size()-pos-1);
	else
		return -1;
		
	mongo::BSONObjBuilder c_bob,r_bob;
	mongo::BSONArrayBuilder sub;
	int index = 0;
	c_bob.append(mkey,kv);
	c_bob.append(arrkey,value.valueary[0].c_str());
//	mongo::BSONObj bobj = m_db.getconn()->findOne(dbcoll.c_str(),c_bob.obj());
//	std::vector<mongo::BSONElement> v;
//	bobj.elems(v);
//	std::string strret = bobj.toString();
//	pos = strret.find(value.valueary[0].c_str());
//	bool c_res = (pos!=-1) ? true:false;
		
	std::string strret;
	bool c_res = true;
	m_mongodb.query(dbcoll,c_bob.obj(),strret);
	if(strret.size() < 10)
		c_res = false;
	else 
		c_res = true;

	mongo::BSONObjBuilder fieldbuild,subbuild,finalbuild;
	mongo::BSONObjBuilder cc_bob;
	cc_bob.append(mkey,kv);
	mongo::Query query;
	
	if(!c_res)
	{
		for(; index < value.valueary.size();index++)
		{
			subbuild.append(value.keyary[index].c_str(),value.valueary[index].c_str());
		}
		fieldbuild.append(subkey,subbuild.obj());
		finalbuild.append("$push",fieldbuild.obj());
	}
	else
	{
		cc_bob.append(arrkey,value.valueary[0].c_str());
		for(index=1;index<value.valueary.size();index++)
		{
			char k[1024]={0};
			sprintf(k,"%s.$.%s",subkey.c_str(),value.keyary[index].c_str());
			subbuild.append(k,value.valueary[index].c_str());
		}
		finalbuild.append("$set",subbuild.obj());
	}
	query = cc_bob.obj();
	m_mongodb.setvalue(dbcoll.c_str(),query,finalbuild.obj(),true);
	return 1;
}

