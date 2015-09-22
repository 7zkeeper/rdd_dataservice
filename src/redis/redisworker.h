/* author 	: zhangqi
 * filename : redisworker.h
 * time		: 2015/09/11
 */ 

#include <string>
#include <iostream>
#include <boost/asio/ip/address.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/threadpool.hpp>

#include <redisclient/redisasyncclient.h>

using namespace boost::threadpool;
static const::std::string redistitle = "push:title:setusercfg";

typedef boost::function<void(bool,const std::string&)> conn_callback;
typedef boost::function<void(const RedisValue& value)> task_callback;
void defTaskFunc(const RedisValue& value);

typedef struct _tag_rediswork_para
{
	std::string 	ip;
	unsigned int 	port;
	unsigned int 	worker;
	std::string		task_title;
	std::string		ret_title;
	std::string 	quitcmd;
}rediswork_para;

class redisworker
{
public:
	redisworker(boost::asio::io_service &ioService,RedisAsyncClient& redisClienti);

	virtual ~redisworker();

	void asynConnect(const conn_callback& conncb);
	virtual void onConnect(bool connected,const std::string& errmsg);
	virtual void onSet(const RedisValue& value);
	virtual void onGet(const RedisValue& value);
	virtual void onLpush(const RedisValue& value);
	virtual void onBrpop(const RedisValue& value);
	virtual void onMessage(const std::vector<char>& bufs);
	
	virtual void task(const RedisValue& value);
	virtual void readIni(std::string file);
	virtual void init(task_callback& task_cb=defTaskFunc);
	void setTitle(std::string title);	
	void seTaskFunc(task_callback& task_cb = defTaskFunc);

	std::string getRetTitle();
private:
	boost::asio::io_service& m_ioService;
	RedisAsyncClient& 	m_redis;	
	rediswork_para		m_para;
	task_callback 		m_taskcb;
	pool				m_pool;
};

