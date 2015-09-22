/* 	author 		: zhangqi
 *  filename	: em_buffer.h
 *  version		: 0.1
 */

#ifndef EM_BUFFER_H_
#define EM_BUFFER_H_

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <endian.h>

inline uint64_t hostToNetwork64(uint64_t host64)
{
  return htobe64(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32)
{
  return htobe32(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16)
{
  return htobe16(host16);
}

inline uint64_t networkToHost64(uint64_t net64)
{
  return be64toh(net64);
}

inline uint32_t networkToHost32(uint32_t net32)
{
  return be32toh(net32);
}

inline uint16_t networkToHost16(uint16_t net16)
{
  return be16toh(net16);
}

static char em_CRLF[] = "\r\n";

class em_buffer
{
private:
	const char* begin() const
	{ return &*m_buffer.begin();}
	
	char* begin() 
	{ return &*m_buffer.begin();}
	
	void makeSpace(size_t len)
	{
		if(writableBytes() + prependableBytes() < len + PREPENDLEN)
		{
			m_buffer.resize(m_writerIndex + len);
		}
		else
		{
			assert(PREPENDLEN < m_readerIndex);
			size_t readable = readableBytes();
			std::copy( 	begin()+m_readerIndex,
						begin()+m_writerIndex,
						begin()+PREPENDLEN);
			m_readerIndex = PREPENDLEN;
			m_writerIndex = m_readerIndex + readable;
			assert(readable == readableBytes());
		}
	}

	std::vector<char> m_buffer;
	size_t m_readerIndex;
	size_t m_writerIndex;

public:
 	static const size_t PREPENDLEN = 8;
	static const size_t INITSIZE = 1024;

	explicit em_buffer(size_t initialSize = INITSIZE)
		: m_buffer(PREPENDLEN + initialSize),
		  m_readerIndex(PREPENDLEN),
		  m_writerIndex(PREPENDLEN)	
	{
		assert(readableBytes() == 0);
		assert(writableBytes() == initialSize);
		assert(prependableBytes() == PREPENDLEN);
	}

	size_t readableBytes() const
	{ return m_writerIndex-m_readerIndex;}

	size_t writableBytes() const
	{ return m_buffer.size() - m_writerIndex;}

	size_t prependableBytes() const
	{ return m_readerIndex;}

	const char* peek() const
	{ return begin() + m_readerIndex;}


	const char* findCRLF() const
	{
		const char* crlf = std::search(peek(),beginWrite(),em_CRLF,em_CRLF+2);
		return crlf == beginWrite() ? NULL : crlf;
	}

	const char* findEOL() const
	{
		const void* eol = memchr(peek(),'\n',readableBytes());
		return static_cast<const char*>(eol);
	}


	void retrieveAll()
	{
		m_readerIndex = PREPENDLEN;
		m_writerIndex = PREPENDLEN;	
	}

	void retrieve(size_t len)
	{
		assert(len <= readableBytes());
		if(len < readableBytes())
		{
			m_readerIndex += len;
		}
		else
		{
			retrieveAll();
		}
	}

	void retrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= beginWrite());
		retrieve(end - peek());
	}

	void retrieveInt64()
	{ retrieve(sizeof(int64_t));}
		
	void retrieveInt32()
	{ retrieve(sizeof(int32_t));}
		 
	void retrieveInt16()
	{ retrieve(sizeof(int16_t));}
	
	void retrieveInt8()
	{ retrieve(sizeof(int8_t));}
	
	void retireveAll()
	{
		m_readerIndex = PREPENDLEN;
		m_writerIndex = PREPENDLEN;
	}

	std::string retrieveAsString(size_t len)
	{
		assert(len <= readableBytes());
		std::string result(peek(),len);
		retrieve(len);
		return result;
	}

	void append(const char* data,size_t len)
	{
		ensureWritableBytes(len);
		std::copy(data,data+len,beginWrite());
		hasWritten(len);
	}

	void append(const void* data,size_t len)
	{ append(static_cast<const char*>(data),len);}

	void ensureWritableBytes(size_t len)
	{
		if(writableBytes() < len)
		{
			makeSpace(len);
		}
		assert(writableBytes() >= len);
	}

	char* beginWrite()
	{ return begin() + m_writerIndex;}

	const char* beginWrite() const
	{ return begin() + m_writerIndex;}

	void hasWritten(size_t len)
	{
		assert(len <= writableBytes());
		m_writerIndex += len;
	}

	void appendInt64(int64_t x)
  	{
    	int64_t be64 = hostToNetwork64(x);
    	append(&be64, sizeof be64);
  	}

   	void appendInt32(int32_t x)
    {
  		int32_t be32 = hostToNetwork32(x);
  		append(&be32, sizeof be32);
  	}
  
    void appendInt16(int16_t x)
    {
    	int16_t be16 = hostToNetwork16(x);
       	append(&be16, sizeof be16);
    }
  
	void appendInt8(int8_t x)
    {
  		append(&x, sizeof x);
  	}
  
    int64_t readInt64()
    {
  		int64_t result = peekInt64();
  		retrieveInt64();
  		return result;
  	}
  
   	int32_t readInt32()
   	{
   		int32_t result = peekInt32();
        retrieveInt32();
        return result;
   	}
  
	int16_t readInt16()
    {
  		int16_t result = peekInt16();
        retrieveInt16();
  	    return result;
    }
  
  	int8_t readInt8()
    {
  		int8_t result = peekInt8();
  	    retrieveInt8();
  		return result;
    }
  
	int64_t peekInt64() const
  	{
  		assert(readableBytes() >= sizeof(int64_t));
  		int64_t be64 = 0;
  		::memcpy(&be64, peek(), sizeof be64);
  		return networkToHost64(be64);
  	}
  
    int32_t peekInt32() const
	{
  		assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
  	    ::memcpy(&be32, peek(), sizeof be32);
        return networkToHost32(be32);
    }
  
	int16_t peekInt16() const
    {
  		assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return networkToHost16(be16);
     }
  
	int8_t peekInt8() const
	{
    	assert(readableBytes() >= sizeof(int8_t));
  	    int8_t x = *peek();
  	    return x;
    }

	void prependInt64(int64_t x)
	{
		int64_t be64 = hostToNetwork64(x);
		prepend(&be64,sizeof be64);
	}


	void prependInt32(int32_t x)
	{
		int32_t be32 = hostToNetwork32(x);
		prepend(&be32,sizeof be32);
	}

	void prependInt16(int16_t x)
	{
		int16_t be16 = hostToNetwork16(x);
		prepend(&be16,sizeof be16);
	}

	void prependInt8(int8_t x)
	{
		prepend(&x,sizeof x);
	}

	void prepend(const void* data,size_t len)
	{
		assert(len <= prependableBytes());
		m_readerIndex -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d,d+len,begin()+m_readerIndex);
	}

};

			
#endif	
	
