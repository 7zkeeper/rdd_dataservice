/* author 	: zhangqi
 * filename : redisworker.cpp
 * time		: 2015/09/11
 */ 

#include "redisworker.h"

typedef boost::function<void(bool,const std::string&)> conn_callback;

redisworker::redisworker(boost::asio::io_service& ioService,RedisAsyncClient& redisClient)
		: m_ioService(ioService),m_redis(redisClient)
{
	m_pool.size_controller().resize(10);
}

redisworker::~redisworker()
{
}

void redisworker::readIni(std::string file)
{
	m_para.ip = "127.0.0.1";
	m_para.port = 6379;
	m_para.title = "push:title:setusercfg";
	m_para.quitcmd = "wanttoquit";
}

void redisworker::setTitle(std::string title)
{
	m_para.title = title;
}


void redisworker::asynConnect(const conn_callback& conncb)
{
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(m_para.ip),m_para.port);
	m_redis.asyncConnect(ep,boost::bind(&redisworker::onConnect,this,_1,_2));
}

void redisworker::onConnect(bool connected,const std::string& errmsg)
{
	if(!connected)
	{
		std::cerr<<"Cannot connect to redis: "<<errmsg<<std::endl;
	}
	else
	{
		std::cout<<"Connected to redis."<<std::endl;
		m_redis.command("BRPOP",m_para.title,"push:test2",0,boost::bind(&redisworker::onBrpop,this,_1));
	}
}	

void redisworker::onSet(const RedisValue& value)
{
	if(value.toString() == "OK")
	{
		std::cout<<"SET: success"<<std::endl;
	}
	else
	{
		std::cerr<<"SET: "<<value.toString()<<std::endl;
	}
}

void redisworker::onGet(const RedisValue& value)
{
	std::cout<<value.toString()<<std::endl;
}

//need argument for the cb
void redisworker::onLpush(const RedisValue& value)
{
	if(value.toInt()>0)
	{
	}
	else
	{
	}
}

void redisworker::onBrpop(const RedisValue& value)
{
/*	if(value.isOk())
	{
		std::cout<<value.toArray()[0].toString()<<std::endl;
		std::cout<<value.toArray()[1].toString()<<std::endl;
		m_redis.command("BRPOP",m_para.title,"push:test2",0,boost::bind(&redisworker::onBrpop,this,_1));
		if(value.toArray()[1].toString()  == m_para.quitcmd)
			m_ioService.stop();
	}
	else
	{
		std::cerr<<"BRPOP: error"<<value.toString()<<std::endl;
	}
*/
	m_pool.schedule(boost::bind(&redisworker::task,this,value));
}


void redisworker::onMessage(const std::vector<char>& bufs)
{
	std::string msg(bufs.begin(),bufs.end());
	std::cerr<<"Message: "<<msg<<std::endl;
	
	if(msg == m_para.quitcmd)
		m_ioService.stop();
}	

void redisworker::task(const RedisValue& value)
{
	if(value.isOk())
	{
		std::cout<<value.toArray()[0].toString()<<std::endl;
		std::cout<<value.toArray()[1].toString()<<std::endl;
		m_redis.command("BRPOP",m_para.title,"push:test2",0,boost::bind(&redisworker::onBrpop,this,_1));
		if(value.toArray()[1].toString()  == m_para.quitcmd)
			m_ioService.stop();
	}
	else
	{
		std::cerr<<"BRPOP: error"<<value.toString()<<std::endl;
	}
}
