#ifndef PROTOCOL_DOT_H
#define PROTOCOL_DOT_H

#include "../engine/socket/ServerSocket.h"
#include "../config/config_types.h"

class CProtocolHandler {
public:
    CProtocolHandler() {}

    virtual bool Handler(void *Buffer, int len, CLIENT_DATA &CData) = 0;

    virtual ~CProtocolHandler() {}
};

class CProxyHandler {
public:
    CProxyHandler() {}

    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData) = 0;

    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData) = 0;

    virtual ~CProxyHandler() {}

};

class CHWirePTHandler : public CProxyHandler {

public:
    CHWirePTHandler() {}

    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "===============CH(up)===================" << endl;
        cout << "Received a Client packet..................... " << endl;
        cout << "Length of Packet is " << len << endl;
        cout << "Packet Type = " << (int) *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.forward_port, Buffer, len, 0);
        return true;
    }

    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "=================CH (down)=================" << endl;
        cout << "Received a Server packet..................... " << endl;
        cout << "Length of Packet is " << len << endl;
        cout << "Packet Type = " << (int) *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.Sh, Buffer, len, 0);
        return true;
    }
};

class CPostgreSQLHandler : public CProxyHandler {

public:
    CPostgreSQLHandler() {}

    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "=============CPostgreSQLHandler(UP)=====================" << endl;
        cout << "Received a Client packet..................... " << endl;

        cout << "Length of Packet is " << len << endl;
        //cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.forward_port, Buffer, len, 0);
        return true;
    }

    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "===============CPostgreSQLHandler(DOWN)===================" << endl;
        cout << "Received a Server packet..................... " << endl;
        cout << "Length of Packet is " << len << endl;
        //cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.Sh, Buffer, len, 0);
        return true;
    }
};

class CMySQLHandler : public CProxyHandler {

public:
    CMySQLHandler() {}

    virtual bool HandleUpstreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "=============CMySQLHandler(UP)=====================" << endl;
        cout << "Received a Client packet..................... " << endl;
        cout << "Length of Packet is " << len << endl;
        // cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.forward_port, Buffer, len, 0);
        return true;
    }

    virtual bool HandleDownStreamData(void *Buffer, int len, CLIENT_DATA &CData) {
        // Log the Content and Forward the Data to the EndPoint
        cout << "===============CMySQLHandler(DOWN)===================" << endl;
        cout << "Received a Server packet..................... " << endl;
        cout << "Length of Packet is " << len << endl;
        // cout << "Packet Type = " <<  (int)  *((unsigned char *) Buffer) << endl;
        cout << "======================================" << endl;
        send(CData.Sh, Buffer, len, 0);
        return true;
    }
};

class PingHandler : public CProtocolHandler {
public:
    PingHandler() {}

    virtual bool Handler(void *Buffer, int len, CLIENT_DATA &CData);

    virtual ~PingHandler() {}

};

class CProtocolSocket : public CServerSocket {
    CProtocolHandler *m_handler = 0;

    static void *ThreadHandler(CProtocolSocket *ptr, void *lptr);

    std::function<void *(void *)> thread_routine_override = 0;
public:

    CProtocolSocket(int port) : CServerSocket(port, "DEFAULT") {
        std::function<void *(void *)> myfunc = [this](void *ptr) -> void * {
            return CProtocolSocket::ThreadHandler(this, ptr);
        };
        thread_routine_override = myfunc;
    }

    bool SetHandler(CProtocolHandler *ph) {
        m_handler = ph;
        return m_handler != 0;
    }

    bool SetPipeline(std::function<void *(CProtocolSocket *, void *)> test_func) {
        std::function<void *(void *)> myfunc = [this, test_func](void *ptr) -> void * {
            return test_func(this, ptr);
        };
        thread_routine_override = myfunc;

        return thread_routine_override != nullptr;
    }

    CProtocolHandler *GetHandler() { return m_handler; }

    bool Start() {
        return Open("<str>", thread_routine_override);
    }

};


class CProxySocket : public CServerSocket {
    CProxyHandler *m_handler = 0;

    static void *ThreadHandler(CProxySocket *ptr, void *lptr);

    std::function<void *(void *)> thread_routine_override = 0;

    RESOLVE_ENDPOINT_RESULT m_configValues;
public:

    CProxySocket(int port) : CServerSocket(port, "DEFAULT") {
        std::function<void *(void *)> myfunc = [this](void *ptr) -> void * {
            return CProxySocket::ThreadHandler(this, ptr);
        };
        thread_routine_override = myfunc;
    }

    bool SetHandler(CProxyHandler *ph) {
        m_handler = ph;
        return m_handler != 0;
    }

    bool SetPipeline(std::function<void *(CProxySocket *, void *)> test_func) {
        std::function<void *(void *)> myfunc = [this, test_func](void *ptr) -> void * {
            return test_func(this, ptr);
        };
        thread_routine_override = myfunc;

        return thread_routine_override != nullptr;
    }

    CProxyHandler *GetHandler() { return m_handler; }

    bool SetConfigValues(RESOLVE_ENDPOINT_RESULT configValues)
    {
        m_configValues = configValues;
        return true;
    }

    RESOLVE_ENDPOINT_RESULT GetConfigValues()
    {
        return m_configValues;
    }

    bool Start() {
        return Open("<str>", thread_routine_override);
    }

};

#endif
