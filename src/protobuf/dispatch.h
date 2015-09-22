/*	author	: zhangqi
 *	filename: dispatcher.h
 *	version	: 0.1
 */

#ifndef _EM_DISPATCHER_H_
#define _EM_DISPATHCER_H_

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <map>

class em_buffer;

typedef boost::function<void()> TimerCallback;
typedef boost::function<void (const void*)> ConnectionCallback;
typedef boost::function<void (const void*)> CloseCallback;
typedef boost::function<void (const void*,em_buffer*)> MessageCallback;

void defaultConnectionCallback(const void* conn);
void defaultMessageCallback(const void* conn,em_buffer buffer);


typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : boost::noncopyable
{
public:
	virtual ~Callback() {}

	virtual void onMessage(const void* ,const MessagePtr& msg) const = 0;
};

template <typename T>
class CallbackT : public Callback
{
public:
	typedef boost::function<void (const void*,const boost::shared_ptr<T>& msg)> ProtobufMsgTCallback;

	CallbackT(const ProtobufMsgTCallback& callback) : m_callback(callback){}

	virtual void onMessage(const void* conn,const MessagePtr& msg) const
	{
		boost::shared_ptr<T> concrete = boost::static_pointer_cast<T>(msg);
		assert(concrete != NULL);
		m_callback(conn,concrete);
	}

private:
	ProtobufMsgTCallback m_callback;
};


class ProtobufDispatcher
{
public:
	typedef boost::function<void (const void*,const MessagePtr& msg)> ProtobufMsgCallback;
	explicit ProtobufDispatcher(const ProtobufMsgCallback& defaultcallback)
						: m_defaultCallback(defaultcallback)
	{
	}

	void onProtobufMessage(const void* conn,const MessagePtr& msg) const
	{
		CallbackMap::const_iterator it = m_callbacks.find(msg->GetDescriptor());
		if(it != m_callbacks.end())
		{
			it->second->onMessage(conn,msg);
		}
		else
		{
			m_defaultCallback(conn,msg);
		}
	}

	template<typename T>
	void registerMessageCallback(const typename CallbackT<T>::ProtobufMsgTCallback& callback)
	{
		boost::shared_ptr<CallbackT<T> > pdesc(new CallbackT<T>(callback));
		m_callbacks[T::descriptor()] = pdesc;
	}
private:
	typedef std::map< const google::protobuf::Descriptor*,boost::shared_ptr<Callback> > CallbackMap;
	CallbackMap m_callbacks;
	ProtobufMsgCallback m_defaultCallback;
};

#endif

