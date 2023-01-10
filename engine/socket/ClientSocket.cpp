
#include "ServerSocket.h"
#include "ClientSocket.h"

//////////////////////////////////////////
//
//
CClientSocket::CClientSocket(char *ServerName ,  int PortNumber) {
    strcpy(m_ServerName,ServerName );
    m_PortNumber = PortNumber;
}
SOCKET CClientSocket::GetSocket() { return m_ConnectSock; }
bool CClientSocket::Resolve()   {

    if (isalpha(m_ServerName[0])) {
        m_HostPointer= gethostbyname(m_ServerName);
    }
    else  { /* Convert nnn.nnn address to a usable one */
        m_addr = inet_addr(m_ServerName);
        m_HostPointer = gethostbyaddr((char *)&m_addr,4,AF_INET);
    }
    if (m_HostPointer == NULL ) { return false; }

    memset(&m_Server,0,sizeof(m_Server));

    memcpy(&(m_Server.sin_addr),m_HostPointer->h_addr,m_HostPointer->h_length);
    m_Server.sin_family = m_HostPointer->h_addrtype;
    m_Server.sin_port = htons(m_PortNumber);
    return true;
}

bool CClientSocket::Connect() {
    m_ConnectSock = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ConnectSock < 0) {
        cout << "Failed To Retrieve Handle" << endl;
        return false;
    }

    if (connect(m_ConnectSock,
                (struct sockaddr *) &m_Server,
                sizeof(m_Server))
        == SOCKET_ERROR) {
        cout << "Failed To Create Socket Client " << endl;
        return false;
    }
    ProtocolHelper::SetReadTimeOut(m_ConnectSock, 1);
    return true;

}

bool CClientSocket::Write( void *buffer , int len ) {
    int RetVal = send(m_ConnectSock,(const char *)buffer,len,0);
    if (RetVal == SOCKET_ERROR) {
        return false;
    }

    return true;
}

bool CClientSocket::Receive( void *buffer , int* len ) {

    int RetVal  = recv(m_ConnectSock ,(char *)buffer,*len,0);


    if ( RetVal == 0 || RetVal == -1)  {
        printf("Error at socket(): %d\n", SocketGetLastError() );
        return false;
    }

    *len = RetVal;
    return true;
}

int CClientSocket::Close() {
    ::CloseSocket(m_ConnectSock);
    return 0;
}



#ifdef  MAIN_INCLUDED

int main(int argc, char **argv )
{

         if ( argc != 4 ) { printf("insufficient command line arguments\n"); return 0; }


         char * server_name = argv[1];
         int port = atoi(argv[2]);
         char * packet_type = argv[3];


         if ( !StartSocket() ){
      		fprintf(stdout,"Failed To Initialize Socket Library\n"); return 1;
        }

        CClientSocket c_sock(server_name, port );
        if (!c_sock.Resolve() ) {
                    printf("Failed to Resolve Client Socket ............\n");
                    return 1;
        }

        if ( !c_sock.Connect() )  {
                    printf("Failed to Connect to the Server ............\n");
                    return 1;
        }
        //========================================================
        PACKET_PDU pd = make_hello();
        if ( strcmp(packet_type,"SELECT") == 0 )
                    pd = make_query();
        else if ( strcmp( packet_type, "INSERT" ) == 0 )
                    pd = make_insert();
        else if ( strcmp(packet_type,"CLOSE" ) == 0 )
                    pd = make_close();
        else if ( strcmp(packet_type,"HELLO" ) == 0 )
         {
                    pd = make_hello();
           }
        else {
                      cout << " Only Select , Insert and Close accepted" << endl;
                      return 0;
         }



        if ( !c_sock.Write(&pd, sizeof(PACKET_PDU)) ){
                        cout << "Failed...to ...Write...To ...." << endl;
                        return 0;
        }
        char buffer[300];
        int rec = sizeof(buffer);
        if (!c_sock.Receive(buffer, &rec) ) {
                   cout << "Failed to Read...." << endl;
                   return 0;
        }
        cout << "Read $ of Bytes => " << rec << endl;
        PACKET_PDU  *input = (PACKET_PDU *) buffer;
        cout << "Read from Socket " << input->buffer << endl;

        //================ Send Message
        c_sock.Close();
        Cleanup();



}

#endif