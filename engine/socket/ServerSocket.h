#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#ifdef WINDOWS_OS
#include <windows.h>
#else // else POSIX
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
typedef struct
{
       SOCKET Sh;             // Socket Handle which represents a Client
       SOCKET forward_port;   // In a non proxying mode, it will be -1
       char remote_addr[32];  //  Address of incoming endpoint
       void *ptr_to_instance; //  Pointer to the instance to which this ClientDATA belongs
       char node_info[255];   //   Node Information
       int mode;              //   R/W mode - Read Or Write
       char ProtocolName[255];
} CLIENT_DATA;

//---------------- Socket Descriptor for Windows
//---------------- Listener Socket => Accepts Connection
//---------------- Incoming Socket is Socket Per Client

//----------------- Thread Entry Points for Listener and Thread Per Client
DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
DWORD WINAPI ClientThreadProc(LPVOID lpParam);
//--------------- Call WSACleanUP for resource de-allocation
void Cleanup();
//------------ Initialize WinSock Library
bool StartSocket();
//-----------------Get Last Socket Error
int SocketGetLastError();
//----------------- Close Socket
int CloseSocket(SOCKET s);

void InitializeLock();

void AcquireLock();
void ReleaseLock();

#else // POSIX
#define SOCKET int
typedef struct
{
    int Sh; // Socket Handle which represents a Client
    int forward_port;
    char remote_addr[32];
    void *ptr_to_instance;
    char node_info[255];
    int mode;
    char ProtocolName[255];
} CLIENT_DATA;

// int InComingSocket;
void Cleanup();
bool StartSocket();

int SocketGetLastError();
int CloseSocket(int s);
#define INVALID_SOCKET (-1)

void *ListenThreadProc(void *lpParameter);
void *ClientThreadProc(void *lpParam);

#define SOCKET_ERROR (-1)

void InitializeLock();
void AcquireLock();

void ReleaseLock();

#endif
//----------------- This is a Structure which will be passed from Socket class to Listen Thread Routine
typedef struct
{
    char node_info[255];   // Encoded Current Node Information as String
    int mode;              // R/W
    void *ptr_to_instance; // Pointer to CServerSocket class
} NODE_INFO;

////////////////////////////////////////////////////
// CServerSocket class
//
class CServerSocket
{
    int m_ProtocolPort = 3500;
    struct sockaddr_in m_LocalAddress;
    char Protocol[255];

public:
    struct sockaddr_in m_RemoteAddress
    {
    };
    SOCKET m_ListnerSocket = -1;
    NODE_INFO info;
    //-------------------------------- Parametrize Thread Routine
    std::function<void *(void *)> thread_routine;

#ifdef WINDOWS_OS
    static DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
    static DWORD WINAPI ClientThreadProc(LPVOID lpParam);
#else
    static void *ListenThreadProc(void *lpParameter);
    static void *ClientThreadProc(void *lpParam);
#endif

    //--------------------- Initialize Local Socket
    CServerSocket(int p_port, string protocol = "DEFAULT");
    //---------------------------- Open Socket
    bool Open(string node_info, std::function<void *(void *)> pthread_routine);
    //------------------- Start a Listening Thread
    bool StartListeningThread(string node_info, std::function<void *(void *)> pthread_routine);
    ///////////////////////////////////////
    //  Close The Server Socket
    //
    bool Close();
    bool Read(char *bfr, int size);
    bool Write(char *bfr, int size);
};

class ProtocolHelper
{
public:
    static string GetIPAddressAsString(struct sockaddr_in *client_addr);
    static string GetIPPortAsString(struct sockaddr_in *client_addr);
    static bool SetReadTimeOut(SOCKET s, long second);
    static bool ReadSocketBuffer(SOCKET s, char *bfr, int size, int *num_read);
    static bool ReadSocketBuffer(SOCKET s, char *bfr, int size);
    static bool WriteSocketBuffer(SOCKET s, char *bfr, int size);
};

#endif
