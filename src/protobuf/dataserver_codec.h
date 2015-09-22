/*	suthor: zhangqi
 *	filename: dataserver_codec.h
 */

#ifndef DATASERVER_CODEC_H_
#define DATASERVER_CODEC_H_


#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <google/protobuf/message.h>
#include "em_buffer.h"


typedef boost::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodec : boost::noncopyable
{
public:
	enum ErrorCode
  	{
    	NOERROR = 0,
    	INVALLEN,
    	CHECKSUMERROR,
    	INVALNAMELEN,
    	UNKNOWNMSGTYPE,
    	PARSEERROR,
  	};

	typedef boost::function<void (const void*, const MessagePtr&)> ProtobufMessageCallback;

  	typedef boost::function<void (const void*,em_buffer*,ErrorCode)> ErrorCallback;

  	explicit ProtobufCodec(const ProtobufMessageCallback& messageCb)
    : m_messageCallback(messageCb),
      m_errorCallback(defaultErrorCallback)
  	{
  	}

 	ProtobufCodec(const ProtobufMessageCallback& messageCb, const ErrorCallback& errorCb)
    	: m_messageCallback(messageCb),
     	m_errorCallback(errorCb)
 	{
  	}

  	void onMessage(const void* conn, em_buffer* buf );

  	void send(const void* conn,const google::protobuf::Message& message)
  	{
   		em_buffer buf;
    	fillEmptyBuffer(&buf, message);
    	//conn->send(&buf);
  	}

	static const std::string& errorCodeToString(ErrorCode errorCode);
    static void fillEmptyBuffer(em_buffer* buf, const google::protobuf::Message& message);
    static google::protobuf::Message* createMessage(const std::string& type_name);
    static MessagePtr parse(const char* buf, int len, ErrorCode* errorCode);
 private:
   	static void defaultErrorCallback(const void*,em_buffer*,ErrorCode);
    ProtobufMessageCallback m_messageCallback;
    ErrorCallback m_errorCallback;
    const static int kHeaderLen = sizeof(int32_t);
    const static int kMinMessageLen = 2*kHeaderLen ; 
    const static int kMaxMessageLen = 64*1024; 
};

#endif  
