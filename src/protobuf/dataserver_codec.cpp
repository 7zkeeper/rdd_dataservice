/* author	: zhangqi
 * filename : dataserver_codec.cpp
 * verson 	: 0.1
 */
#include <google/protobuf/descriptor.h>
#include "dataserver_codec.h"


int32_t asInt32(const char* buf)
{
	int32_t val32 = 0;
	::memcpy(&val32,buf,sizeof(int32_t));
	return networkToHost32(val32);
}

void ProtobufCodec::fillEmptyBuffer(em_buffer* buf,const google::protobuf::Message& msg)
{
	assert(buf->readableBytes() == 0);
	
	const std::string& typeName = msg.GetTypeName();
	int32_t nameLen = static_cast<int32_t>(typeName.size()+1);
	buf->appendInt32(nameLen);
	buf->append(typeName.c_str(),nameLen);

	int byte_size = msg.ByteSize();
	buf->ensureWritableBytes(byte_size);

	uint8_t* start = reinterpret_cast<uint8_t*>(buf->beginWrite());
	uint8_t* end = msg.SerializeWithCachedSizesToArray(start);

	if((end - start) != byte_size)
	{
//		ByteSizeConsistencyError(byte_size,msg.ByteSize(),static_cast<int>(end-start));
	}
	buf->hasWritten(byte_size);
	
	assert(buf->readableBytes() == sizeof nameLen + nameLen + byte_size);
	int32_t len =  hostToNetwork32(static_cast<int32_t>(buf->readableBytes()));
	buf->prepend(&len,sizeof len);
}

const std::string NOERRORStr = "NoError";
const std::string INVALLENStr = "InvalidLength";
const std::string INVALNAMELENStr = "InvalidNameLen";
const std::string UNKNOWNMSGTYPEStr = "UnknownMessageType";
const std::string PARSEERRORStr = "ParseError";
const std::string UNKNOWNERRORStr = "UnknownError";

const std::string& ProtobufCodec::errorCodeToString(ErrorCode err)
{
	switch(err)
	{
	case NOERROR:
		return NOERRORStr;
	case INVALLEN:
		return INVALLENStr;
	case INVALNAMELEN:
		return INVALNAMELENStr;
	case UNKNOWNMSGTYPE:
		return UNKNOWNMSGTYPEStr;
	case PARSEERROR:
		return PARSEERRORStr;
	default:
		return UNKNOWNERRORStr;
	}
}

void ProtobufCodec::defaultErrorCallback(const void* conn,em_buffer* buf,ErrorCode err)
{
//	printf("%s\n",errorCodeToString(err));
}

void ProtobufCodec::onMessage(const void* conn,em_buffer* buf)
{
	while(buf->readableBytes() >= kMinMessageLen + kHeaderLen)
	{
		const int32_t len = buf->peekInt32();
		if(len > kMaxMessageLen || len < kMinMessageLen)
		{
			m_errorCallback(conn,buf,INVALLEN);
			break;
		}

		else if (buf->readableBytes() >= len+kHeaderLen)
		{
			ErrorCode err = NOERROR;
			MessagePtr msg = parse(buf->peek()+kHeaderLen,len,&err);
			if(err == NOERROR && msg)
			{
				m_messageCallback(conn,msg);
				buf->retrieve(kHeaderLen+len);
			}
			else
			{
				m_errorCallback(conn,buf,err);
				break;
			}	
		}
		else
		{
			break;
		}
	}
}

google::protobuf::Message* ProtobufCodec::createMessage(const std::string& typeName)
{
	google::protobuf::Message* msg = NULL;
	const google::protobuf::Descriptor* descriptor =
			google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if(descriptor)
	{
		const google::protobuf::Message* prototype = 
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if(prototype)
		{
			msg = prototype->New();
		}
	}
	return msg;
}

MessagePtr ProtobufCodec::parse(const char* buf,int len ,ErrorCode* err)
{
	MessagePtr msg;
	int32_t nameLen = asInt32(buf);
	if(nameLen >= 2 && nameLen <= len-2*kHeaderLen)
	{
		std::string typeName(buf+kHeaderLen,buf+kHeaderLen+nameLen-1);
		msg.reset(createMessage(typeName));
	
		if(msg)
		{
			const char* data = buf + kHeaderLen + nameLen;
			int32_t dataLen = len - nameLen - kHeaderLen;
			if(msg->ParseFromArray(data,dataLen))
			{
				*err = NOERROR;
			}
			else
			{
				*err = PARSEERROR;
			}
		}	
		else
		{
			*err = UNKNOWNMSGTYPE;
		}
	}
	else
	{
		*err = INVALNAMELEN;
	}
	return msg;
}
	
