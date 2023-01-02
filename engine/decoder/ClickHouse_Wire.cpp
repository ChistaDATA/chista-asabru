#ifdef WINDOWS_OS
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#define SOCKET int

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

using namespace std;

#include "ClickHouse_Wire.h"

 ClickReadProtoBuffer::ClickReadProtoBuffer(void *buffer, int len)
  {
    m_buffer = new char[len];
    memcpy(m_buffer, buffer, len);
    m_len = len;
    m_index = 0;
  }
  ClickReadProtoBuffer::~ClickReadProtoBuffer()
  {
    delete m_buffer;
    m_len = 0;
  }

  bool  ClickReadProtoBuffer::ReadByte(unsigned char *byte)
  {
    *byte = m_buffer[m_index++];
    return m_index < m_len;
  }
  bool  ClickReadProtoBuffer::ReadBuffer(void *buff, int size)
  {
    memcpy(buff, &m_buffer[m_index], size);
    m_index += size;
    return m_index < m_len;
  }
  bool  ClickReadProtoBuffer::ReadVarint64(uint64_t *value)
  {
    *value = 0;
    for (size_t i = 0; i < MAX_VARINT_BYTES; ++i)
    {
      uint8_t byte = 0;
      if (!ReadByte(&byte))
      {
        return false;
      }
      else
      {
        *value |= uint64_t(byte & 0x7F) << (7 * i);
        if (!(byte & 0x80))
        {
          return true;
        }
      }
    }

    // TODO skip invalid
    return false;
  }
  bool  ClickReadProtoBuffer::ReadAll(void *buf, size_t len)
  {
    return ReadBuffer(buf, len);
  }
  bool  ClickReadProtoBuffer::ReadString(std::string *value)
  {
    uint64_t len = 0;
    if (ReadVarint64(&len))
    {
      if (len > 0x00FFFFFFULL)
      {
        return false;
      }
      value->resize((size_t)len);
      return ReadAll(value->data(), (size_t)len);
    }

    return false;
  }
  template <typename T>
  inline bool  ClickReadProtoBuffer::ReadFixed(T *value)
  {
    return ReadAll(value, sizeof(T));
  }




CProtocolPacketDecoder::CProtocolPacketDecoder(char *buffer, int len){
    m_buffer = new ClickReadProtoBuffer(buffer, len);
}

  uint64_t CProtocolPacketDecoder::GetPacketType()
  {
    uint64_t packet_type;
    if (!m_buffer->ReadVarint64(&packet_type))
    {
      cout << "Wrong ............packettype " << endl;
      return -1;
    }
    cout << "Received From Clickhouse Client......" << packet_type << endl;
    return packet_type;
  }

  bool CProtocolPacketDecoder::DecidePacketType()
  {
    uint64_t packet_type;
    if (!m_buffer->ReadVarint64(&packet_type))
    {
      cout << "Wrong ............packettype " << endl;
      return false;
    }
    cout << "Received From Clickhouse Client......" << packet_type << endl;
    return 0;
  }

  bool CProtocolPacketDecoder::DecodeQueryPacket()
  {
    uint64_t packet_type;
    if (!m_buffer->ReadVarint64(&packet_type))
    {
      cout << "Wrong ............packettype " << endl;
      return false;
    }
    cout << "Received From Clickhouse Client......" << packet_type << endl;
    std::string query_id;
    if (!m_buffer->ReadString(&query_id))
    {
      cout << "Wrong ............packettype " << endl;
      return false;
    }
    cout << "Query id = " << query_id << endl;

    /*

                if (server_info_.revision >= DBMS_MIN_REVISION_WITH_CLIENT_INFO) {
                          string temp;
                   if ( !m_buffer->ReadString(&temp) ){
                             cout << "Wrong ............packettype " << endl;
                             return false;
                  }
                  cout << "Marker = " << temp<< endl;

                }
                */

    string temp;

    uint64_t temp_num;
    m_buffer->ReadVarint64(&temp_num);
    cout << "Stages = " << endl;
    m_buffer->ReadVarint64(&temp_num);
    cout << "Compression = " << temp_num << endl;
    m_buffer->ReadString(&temp);
    cout << "Query String = " << temp << endl;
    return 0;
  }

  void CProtocolPacketDecoder::DecodeClientPacket(uint64_t packet_type)
  {
    switch (packet_type)
    {
    case 0: // Hello packet
      DecodeClientHelloPacket();
      break;

    default:
      cout << "Invalid Client Packet Type" << endl;
      break;
    }
  }

  bool CProtocolPacketDecoder::DecodeClientHelloPacket()
  {
    ClientHelloPacket hello;

    cout << "Decoding Client Hello Packet.." << endl;

    m_buffer->ReadString(&hello.client_name);
    cout << "Client Name " << hello.client_name << endl;

    m_buffer->ReadVarint64(&hello.version_major);
    cout << "Major Version " << hello.version_major << endl;

    m_buffer->ReadVarint64(&hello.version_minor);
    cout << "Minor Version " << hello.version_minor << endl;

    m_buffer->ReadVarint64(&hello.protocol_version);
    cout << "Protocol Version " << hello.protocol_version << endl;

    m_buffer->ReadString(&hello.database);

    m_buffer->ReadString(&hello.username);

    m_buffer->ReadString(&hello.password);

    return true;
  }

  bool CProtocolPacketDecoder::DecodeHelloPacket()
  {
    uint64_t packet_type;
    if (!m_buffer->ReadVarint64(&packet_type))
    {
      cout << "Wrong ............packettype " << endl;
      return false;
    }
    cout << "Received From Clickhouse Client......" << packet_type << endl;
    if (packet_type != ClientCodes::Hello)
    {
      cout << "Expected Hello Packet......" << endl;
      return false;
    }
    std::string client_name;
    cout << "Received ...Hello Packet....." << endl;
    if (!m_buffer->ReadString(&client_name))
    {
      return false;
    }
    cout << client_name << endl;
    return true;
  }

 CProtocolPacketDecoder:: ~CProtocolPacketDecoder() { delete m_buffer; }

  void CProtocolPacketDecoder::DecodeServerPackets(uint64_t packet_type)
  {
    switch (packet_type)
    {
    case 0: // Hello packet
      DecodeServerHelloPacket();
      break;
    default:
      cout << "Invalid Server Packet Type" << endl;
      break;
    }
  }

  bool CProtocolPacketDecoder::DecodeServerHelloPacket()
  {
    ServerHelloPacket hello;

    cout << "Decoding Server Hello Packet.." << endl;

    m_buffer->ReadString(&hello.name);
    cout << "Name " << hello.name << endl;

    m_buffer->ReadVarint64(&hello.version_major);
    cout << "Major Version " << hello.version_major << endl;

    m_buffer->ReadVarint64(&hello.version_minor);
    cout << "Minor Version " << hello.version_minor << endl;

    m_buffer->ReadVarint64(&hello.revision);
    cout << "Protocol Version " << hello.revision << endl;

    m_buffer->ReadString(&hello.tz);
    cout << "Protocol Version " << hello.tz << endl;

    m_buffer->ReadString(&hello.display_name);
    cout << "Display Name " << hello.display_name << endl;

    m_buffer->ReadVarint64(&hello.version_patch);
    cout << "Version Patch " << hello.version_patch << endl;

    return true;
  }


 ClickWriteProtoBuffer::ClickWriteProtoBuffer(void *buffer, int len){
    m_buffer = new char[len];
    memcpy(m_buffer, buffer, len);
    m_len = len;
    m_index = 0;
  }
  ClickWriteProtoBuffer::~ClickWriteProtoBuffer(){
    delete m_buffer;
    m_len = 0;
  }

  bool ClickWriteProtoBuffer::GetBuffer(void *buffer, int *len)
  {
    memcpy(buffer, m_buffer, m_index);
    *len = m_index;
    return true;
  }
  void ClickWriteProtoBuffer::WriteBuffer(void *buff, int len)
  {

    memcpy(&m_buffer[m_index], buff, len);
    m_index += len;
  }
  template <typename T>
  inline void ClickWriteProtoBuffer::WriteFixed(const T &value)
  {
    WriteAll(&value, sizeof(T));
  }

  inline void ClickWriteProtoBuffer::WriteBytes(const void *buf, size_t len)
  {
    WriteAll(buf, len);
  }

  inline void ClickWriteProtoBuffer::WriteString(std::string value)
  {
    WriteVarint64(value.size());
    WriteAll(value.data(), value.size());
  }

  inline void ClickWriteProtoBuffer::WriteUInt64(const uint64_t value)
  {
    WriteVarint64(value);
  }

  void ClickWriteProtoBuffer::WriteAll(const void *buf, size_t len)
  {
    const size_t original_len = len;
    const uint8_t *p = static_cast<const uint8_t *>(buf);
    return WriteBuffer((void *)p, len);
  }

  void ClickWriteProtoBuffer::WriteVarint64(uint64_t value)
  {
    uint8_t bytes[MAX_VARINT_BYTES];
    int size = 0;
    for (size_t i = 0; i < MAX_VARINT_BYTES; ++i)
    {
      uint8_t byte = value & 0x7F;
      if (value > 0x7F)
        byte |= 0x80;

      bytes[size++] = byte;

      value >>= 7;
      if (!value)
      {
        break;
      }
    }

    WriteAll(bytes, size);
  }



  
  CProtocolPacketEncoder::CProtocolPacketEncoder(char *buffer, int len)
  {
    m_buffer = new ClickWriteProtoBuffer(buffer, len);
  }

  bool 
  CProtocolPacketEncoder::DispatchBufferForSocket(SOCKET sock)
  {
    char buffer_ptr[32000];
    int num_read = 0;
    m_buffer->GetBuffer(buffer_ptr, &num_read);
    send(sock, buffer_ptr, num_read, 0);
    cout << "Sending ....." << num_read << "  bytes " << endl;
    char *buffer = buffer_ptr;
    int index = 0;
    while (index < num_read)
    {
      cout << *buffer++;
      index++;
    }
    cout << endl;
    return true;
  }

  bool 
  CProtocolPacketEncoder::EncodeHelloPacket()
  {
    ServerInfo info;
    /*struct ServerInfo {
	std::string name;
	std::string timezone;
	std::string display_name;
	uint64_t    version_major;
	uint64_t    version_minor;
	uint64_t    version_patch;
	uint64_t    revision;
	}; */

    info.name = "ClickHouse server";
    info.timezone = "Israel";
    info.display_name = "Server version 22.9";
    info.version_major = DBMS_VERSION_MAJOR;
    info.version_minor = DBMS_VERSION_MAJOR;
    info.version_patch = 1;
    info.revision = REVISION;
    //--------------- Write Hello
    m_buffer->WriteVarint64(ServerCodes ::Hello);
    m_buffer->WriteString(info.name);
    m_buffer->WriteUInt64(info.version_major);
    m_buffer->WriteUInt64(info.version_minor);
    m_buffer->WriteUInt64(info.revision);
    if (info.revision >= DBMS_MIN_REVISION_WITH_SERVER_TIMEZONE)
    {
      m_buffer->WriteString(info.timezone);
    }
    if (info.revision >= DBMS_MIN_REVISION_WITH_SERVER_DISPLAY_NAME)
    {
      m_buffer->WriteString(info.display_name);
    }
    if (info.revision >= DBMS_MIN_REVISION_WITH_VERSION_PATCH)
    {
      m_buffer->WriteUInt64(info.version_patch);
    }

    cout << "Protocol Encoding for Client Over ..................." << endl;
    return true;
  }
  
  CProtocolPacketEncoder::~CProtocolPacketEncoder() { delete m_buffer; }

