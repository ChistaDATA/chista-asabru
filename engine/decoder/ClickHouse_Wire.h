#ifndef CLICKHOUSE_WIRE_DOT_H
#define CLICKHOUSE_WIRE_DOT_H
//------------------- Clickhouse Prtocol Specfic Code
#define DBMS_NAME "ClickHouse"
#define DBMS_VERSION_MAJOR 2
#define DBMS_VERSION_MINOR 1

#define DBMS_MIN_REVISION_WITH_TEMPORARY_TABLES 50264
#define DBMS_MIN_REVISION_WITH_TOTAL_ROWS_IN_PROGRESS 51554
#define DBMS_MIN_REVISION_WITH_BLOCK_INFO 51903
#define DBMS_MIN_REVISION_WITH_CLIENT_INFO 54032
#define DBMS_MIN_REVISION_WITH_SERVER_TIMEZONE 54058
#define DBMS_MIN_REVISION_WITH_QUOTA_KEY_IN_CLIENT_INFO 54060
//#define DBMS_MIN_REVISION_WITH_TABLES_STATUS            54226
#define DBMS_MIN_REVISION_WITH_TIME_ZONE_PARAMETER_IN_DATETIME_DATA_TYPE 54337
#define DBMS_MIN_REVISION_WITH_SERVER_DISPLAY_NAME 54372
#define DBMS_MIN_REVISION_WITH_VERSION_PATCH 54401
#define DBMS_MIN_REVISION_WITH_LOW_CARDINALITY_TYPE 54405

#define REVISION DBMS_MIN_REVISION_WITH_LOW_CARDINALITY_TYPE

namespace ServerCodes
{
  enum
  {
    Hello = 0,                /// Name, version, revision.
    Data = 1,                 /// `Block` of data, may be compressed.
    Exception = 2,            /// Exception that occured on server side during query execution.
    Progress = 3,             /// Query execcution progress: rows and bytes read.
    Pong = 4,                 /// response to Ping sent by client.
    EndOfStream = 5,          /// All packets were sent.
    ProfileInfo = 6,          /// Profiling data
    Totals = 7,               /// Block of totals, may be compressed.
    Extremes = 8,             /// Block of mins and maxs, may be compressed.
    TablesStatusResponse = 9, /// Response to TableStatus.
    Log = 10,                 /// Query execution log.
  };
}

/// Types of packets sent by client.
namespace ClientCodes
{
  enum
  {
    Hello = 0,  /// Name, version, default database name.
    Query = 1,  /** Query id, query settings, query processing stage,
                 * compression status, and query text (no INSERT data).
                 */
    Data = 2,   /// Data `Block` (e.g. INSERT data), may be compressed.
    Cancel = 3, /// Cancel query.
    Ping = 4,   /// Check server connection.
  };
}

struct ServerInfo
{
  std::string name;
  std::string timezone;
  std::string display_name;
  uint64_t version_major;
  uint64_t version_minor;
  uint64_t version_patch;
  uint64_t revision;
};

////////////////////////
// Client Packets
struct ClientHelloPacket
{
  std::string client_name;
  uint64_t version_major;
  uint64_t version_minor;
  uint64_t protocol_version;
  std::string database;
  std::string username;
  std::string password;
};

///////////////////////////
// Srever Packets
struct ServerHelloPacket
{
  std::string name;
  uint64_t version_major;
  uint64_t version_minor;
  uint64_t revision;
  std::string tz;
  std::string display_name;
  uint64_t version_patch;
};

const int MAX_VARINT_BYTES = 10;
class ClickReadProtoBuffer{
  char *m_buffer = 0;
  int m_index = -1;
  int m_len;

public:
  ClickReadProtoBuffer(void *buffer, int len);
  ~ClickReadProtoBuffer();
  bool ReadByte(unsigned char *byte);
  bool ReadBuffer(void *buff, int size);
  bool ReadVarint64(uint64_t *value);
  bool ReadAll(void *buf, size_t len);
  bool ReadString(std::string *value);
  template <typename T>
  inline bool ReadFixed(T *value);
};

class CProtocolPacketDecoder{
  ClickReadProtoBuffer *m_buffer = 0;

public:
  CProtocolPacketDecoder(char *buffer, int len);
  CProtocolPacketDecoder();
  uint64_t GetPacketType();
  bool DecidePacketType();
  bool DecodeQueryPacket();
  void DecodeClientPacket(uint64_t packet_type);
  bool DecodeClientHelloPacket();
  bool DecodeHelloPacket();
  ~CProtocolPacketDecoder();
  void DecodeServerPackets(uint64_t packet_type);
  bool DecodeServerHelloPacket();
 
};
///////////////////////////////////////////
// A Basic Buffer for Protocol Decoding
//
class ClickWriteProtoBuffer{
  char *m_buffer = 0;
  int m_index = -1;
  int m_len;

public:
  ClickWriteProtoBuffer(void *buffer, int len);
  ~ClickWriteProtoBuffer();
  bool GetBuffer(void *buffer, int *len);
  void WriteBuffer(void *buff, int len);
  template <typename T>
  inline void WriteFixed(const T &value);
  inline void WriteBytes(const void *buf, size_t len);
  inline void WriteString(std::string value);
  inline void WriteUInt64(const uint64_t value);
  void WriteAll(const void *buf, size_t len);
  void WriteVarint64(uint64_t value);
  
};

class CProtocolPacketEncoder{
  ClickWriteProtoBuffer *m_buffer = 0;
public:
  CProtocolPacketEncoder(char *buffer, int len);
  bool DispatchBufferForSocket(SOCKET sock);
  bool EncodeHelloPacket();
  ~CProtocolPacketEncoder() ;
};


#endif
