#ifndef PROTOCOL_SOCKET_DOT_H
#define PROTOCOL_SOCKET_DOT_H

#include "ServerSocket.h"
#include "CProxyHandler.h"
#include "CProtocolHandler.h"
#include "../config/config_types.h"

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

#endif
