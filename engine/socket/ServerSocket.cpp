// ServerSocket.cpp - The Implementation of ServerSocket class
//
// https://github.com/eminfedar/async-sockets-cpp
//
//

#include "ServerSocket.h"

#ifdef WINDOWS_OS
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define DWORD unsigned long

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <functional>
using namespace std;

////////////////////////////////

#ifdef WINDOWS_OS

//---------------- Socket Descriptor for Windows
//---------------- Listener Socket => Accepts Connection
//---------------- Incoming Socket is Socket Per Client

//----------------- Thread Entry Points for Listener and Thread Per Client
DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
DWORD WINAPI ClientThreadProc(LPVOID lpParam);
//--------------- Call WSACleanUP for resource de-allocation
void Cleanup()
{
    WSACleanup();
}
//------------ Initialize WinSock Library
bool StartSocket()
{
    WORD Ver;
    WSADATA wsd;
    Ver = MAKEWORD(2, 2);
    if (WSAStartup(Ver, &wsd) == SOCKET_ERROR)
    {
        WSACleanup();
        return false;
    }
    return true;
}
//-----------------Get Last Socket Error
int SocketGetLastError() { return WSAGetLastError(); }
//----------------- Close Socket
int CloseSocket(SOCKET s)
{
    closesocket(s);
    return 0;
}

/* This is the critical section object (statically allocated). */
CRITICAL_SECTION m_CriticalSection;

void InitializeLock()
{
    InitializeCriticalSection(&m_CriticalSection);
}

void AcquireLock()
{
    EnterCriticalSection(&m_CriticalSection);
}
void ReleaseLock()
{
    LeaveCriticalSection(&m_CriticalSection);
}

#else // POSIX
#define SOCKET int

// int InComingSocket;
void Cleanup() {}
bool StartSocket() { return true; }

int SocketGetLastError() { return 0xFFFF; }
int CloseSocket(int s)
{
    shutdown(s, 2);
    return 0;
}
#define INVALID_SOCKET (-1)

void *ListenThreadProc(void *lpParameter);
void *ClientThreadProc(void *lpParam);

#define SOCKET_ERROR (-1)

#if defined(__APPLE__)
/* This is the critical section object (statically allocated). */
static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

void InitializeLock()
{
}
void AcquireLock()
{
    /* Enter the critical section -- other threads are locked out */
    pthread_mutex_lock(&cs_mutex);
}
void ReleaseLock()
{
    /*Leave the critical section -- other threads can now pthread_mutex_lock()  */
    pthread_mutex_unlock(&cs_mutex);
}

#endif
//--------------------- Initialize Local Socket
CServerSocket::CServerSocket(int p_port, string protocol) : m_ProtocolPort(p_port)
{
    m_LocalAddress.sin_family = AF_INET;
    m_LocalAddress.sin_addr.s_addr = INADDR_ANY;
    m_LocalAddress.sin_port = htons(m_ProtocolPort);
    strcpy(Protocol, protocol.c_str());
}
//---------------------------- Open Socket
bool CServerSocket::Open(string node_info, std::function<void *(void *)> pthread_routine)
{
    // Fetch Socket Descriptor, Bind , Listen and Start Listening Thread
    if ((m_ListnerSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        cout << "Failed to create Socket Descriptor " << endl;
        return false;
    }

    if (::bind(m_ListnerSocket, (struct sockaddr *)&m_LocalAddress, sizeof(m_LocalAddress)) == SOCKET_ERROR)
    {
        cout << "Failed to Bind" << endl;
        return false;
    }

    if (listen(m_ListnerSocket, 5) == SOCKET_ERROR)
    {
        cout << "Listening Socket Failed.. ...." << endl;
        return false;
    }
    return StartListeningThread(node_info, pthread_routine);
}
//------------------- Start a Listening Thread
bool CServerSocket::StartListeningThread(string node_info, std::function<void *(void *)> pthread_routine)
{

    strcpy(info.node_info, node_info.c_str());
    cout << "\nThread  => " << info.node_info << endl;
    info.mode = 1;
    this->thread_routine = pthread_routine;
    info.ptr_to_instance = (void *)this;

    if (info.ptr_to_instance == 0)
    {
        return false;
    }
#ifdef WINDOWS_OS
    DWORD Thid;

    CreateThread(NULL, 0, CServerSocket::ListenThreadProc, (void *)&info, 0, &Thid);
#else
    pthread_t thread1;
    int iret1;
    //-----

    iret1 = pthread_create(&thread1, NULL, CServerSocket::ListenThreadProc, (void *)&info);

#endif
    cout << "Started Listening Thread :" << endl;
    return true;
}
///////////////////////////////////////
//  Close The Server Socket
//
bool CServerSocket::Close()
{
    CloseSocket(m_ListnerSocket);
    return false;
}

bool CServerSocket::Read(char *bfr, int size)
{
    int RetVal = recv(m_ListnerSocket, bfr, size, 0);
    if (RetVal == 0 || RetVal == -1)
    {
        return false;
    }
    return true;
}
bool CServerSocket::Write(char *bfr, int size)
{
    int bytes_send = send(m_ListnerSocket, (char *)bfr, size, 0);
    return bytes_send > 0;
}
string ProtocolHelper::GetIPAddressAsString(struct sockaddr_in *client_addr)
{
    struct sockaddr_in *pV4Addr = client_addr;
    struct in_addr ipAddr = pV4Addr->sin_addr;
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddr, str, INET_ADDRSTRLEN);
    return string(str);
}

string ProtocolHelper::GetIPPortAsString(struct sockaddr_in *client_addr)
{
    struct sockaddr_in *pV4Addr = client_addr;
    in_port_t ipPort = pV4Addr->sin_port;
    return std::to_string(ipPort);
}

bool ProtocolHelper::SetReadTimeOut(SOCKET s, long second)
{
    struct timeval tv;
    tv.tv_sec = second;
    tv.tv_usec = 0;
    int timeoutVal = 0;
    int timeoutValSizeInInt = sizeof(int);
    int timeoutValSizeInTimeVal = sizeof(timeval);
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&tv, timeoutValSizeInTimeVal) != SOCKET_ERROR)
    {
        return true;
    }
    return false;
}

bool ProtocolHelper::ReadSocketBuffer(SOCKET s, char *bfr, int size, int *num_read)
{
    int RetVal = recv(s, bfr, size, 0);
    if (RetVal == 0 || RetVal == -1)
    {
        *num_read = RetVal;
        return false;
    }
    *num_read = RetVal;
    return true;
}
bool ProtocolHelper::ReadSocketBuffer(SOCKET s, char *bfr, int size)
{
    int RetVal = recv(s, bfr, size, 0);
    if (RetVal == 0 || RetVal == -1)
    {
        return false;
    }

    return true;
}
bool ProtocolHelper::WriteSocketBuffer(SOCKET s, char *bfr, int size)
{
    int RetVal = send(s, bfr, size, 0);
    if (RetVal == 0 || RetVal == -1)
        return false;
    return true;
}

/**
 * Creates a thread that listens to incoming connections to the socket
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ListenThreadProc(
    LPVOID lpParameter)
#else
void *CServerSocket::ListenThreadProc(
    void *lpParameter)
#endif
{

    printf("Entered the Listener Thread :\n");
    NODE_INFO info;
    memcpy(&info, lpParameter, sizeof(NODE_INFO));
    CServerSocket *curr_instance = (CServerSocket *)(info.ptr_to_instance);
    cout << "node info => " << string(info.node_info) << endl;
    if (curr_instance == 0)
    {
        cout << "Failed to Retrieve Pointers" << endl;
        return 0;
    }
    while (1)
    {
        cout << "started client thread loop :\n"
             << endl;
        unsigned int Len = sizeof(curr_instance->m_RemoteAddress);

#ifdef WINDOWS_OS
        SOCKET InComingSocket = accept(curr_instance->m_ListnerSocket,
                                       (struct sockaddr *)&(curr_instance->m_RemoteAddress), (int *)&Len);
#else
        SOCKET InComingSocket = accept(curr_instance->m_ListnerSocket,
                                       (struct sockaddr *)&(curr_instance->m_RemoteAddress), (socklen_t *)&Len);

#endif
        printf("\n....................After the Accept........\n");
        if (InComingSocket == INVALID_SOCKET)
        {
            fprintf(stderr, "accept error %d\n", SocketGetLastError());
            Cleanup();
            return 0;
        }
        printf("\n....................Accepted a new Connection........\n");
        CLIENT_DATA ClientData;
        DWORD ThreadId;
        ClientData.Sh = InComingSocket;
        memcpy(ClientData.node_info, info.node_info, 255);
        cout << "Before callint Client Thread => "
             << "ClientData.node_info" << endl;
        ClientData.mode = info.mode;
        string remote_ip = ProtocolHelper::GetIPAddressAsString(&(curr_instance->m_RemoteAddress));
        strcpy(ClientData.remote_addr, remote_ip.c_str());
        cout << "Remote IP address : " << remote_ip << endl;
        string remote_port = ProtocolHelper::GetIPPortAsString(&(curr_instance->m_RemoteAddress));
        cout << "Remote port : " << remote_port << endl;
        ClientData.ptr_to_instance = curr_instance;

#ifdef WINDOWS_OS

        // ClientData.Sh = InComingSocket;
        ::CreateThread(NULL, 0, CServerSocket::ClientThreadProc, (LPVOID)&ClientData,
                       0, &ThreadId);
#else
        pthread_t thread2;
        // ClientData.Sh = InComingSocket;
        pthread_create(&thread2, NULL, CServerSocket::ClientThreadProc, (void *)&ClientData);
#endif
    }

    return 0;
}

/**
 * Creates a client thread procedure that handles incoming and outgoing
 * messages to the socket. It offloads the work to the handler
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ClientThreadProc(
    LPVOID lpParam)
#else
void *CServerSocket::ClientThreadProc(
    void *lpParam)
#endif
{

    CLIENT_DATA CData;
    memcpy(&CData, lpParam, sizeof(CLIENT_DATA));
    ((CServerSocket *)CData.ptr_to_instance)->thread_routine(lpParam);
    return 0;
}
