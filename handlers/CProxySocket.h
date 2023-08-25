/***********************************************************************************************
 * File: ProxySocket.h
 * Description: This header file defines the CProxySocket class, which is derived from CServerSocket.
 *              CProxySocket is used for managing proxy server functionality.
 ***********************************************************************************************/

#ifndef PROXY_SOCKET_DOT_H
#define PROXY_SOCKET_DOT_H

#include "ServerSocket.h"           // Include the base class header
#include "CProxyHandler.h"          // Include the proxy handler header
#include "../config/config_types.h" // Include the configuration types header

// Class definition for CProxySocket
class CProxySocket : public CServerSocket
{
    CProxyHandler *m_handler = nullptr; // Pointer to the proxy handler instance
    RESOLVE_ENDPOINT_RESULT m_configValues; // Configuration values for the proxy socket

    // Static thread handler function for internal use
    static void *ThreadHandler(CProxySocket *ptr, void *lptr);

    // Function pointer for the thread routine
    std::function<void *(void *)> thread_routine_override = nullptr;

public:
    // Constructor: Initializes the CProxySocket instance
    explicit CProxySocket(int port) : CServerSocket(port, "DEFAULT")
    {
        // Create a lambda function that wraps the ThreadHandler
        std::function<void *(void *)> pipelineLambda = [this](void *ptr) -> void *
        {
            return CProxySocket::ThreadHandler(this, ptr);
        };
        thread_routine_override = pipelineLambda; // Set the thread routine
    }

    // Set the proxy handler for the socket
    bool SetHandler(CProxyHandler *ph)
    {
        m_handler = ph;
        return m_handler != nullptr;
    }

    // Set a custom pipeline function for the socket
    bool SetPipeline(std::function<void *(CProxySocket *, void *)> pipelineFunction)
    {
        // Create a lambda that wraps the provided pipeline function
        std::function<void *(void *)> pipelineLambda = [this, pipelineFunction](void *ptr) -> void *
        {
            return pipelineFunction(this, ptr);
        };
        thread_routine_override = pipelineLambda; // Set the thread routine to the pipeline

        return thread_routine_override != nullptr;
    }

    // Get the proxy handler instance
    CProxyHandler *GetHandler() { return m_handler; }

    // Set configuration values for the proxy socket
    bool SetConfigValues(RESOLVE_ENDPOINT_RESULT configValues)
    {
        m_configValues = configValues;
        return true;
    }

    // Get configuration values for the proxy socket
    RESOLVE_ENDPOINT_RESULT GetConfigValues()
    {
        return m_configValues;
    }

    // Start the proxy socket
    bool Start(string identifier)
    {
        return Open(identifier, thread_routine_override); // Call the base class's Open function
    }
};

#endif // PROXY_SOCKET_DOT_H
