#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <string>
#include <iostream>
#include <thread>
#include <list>
using namespace std;

#include "handlers/CProtocolServer.h"
#include "handlers/CHttpHandler.h"
#include "config/config.h"
#include "config/ConfigSingleton.h"


typedef struct
{
    char node_end_point[40];
    int port;
}NODE_PING_INFO;
void PerformLogic() {}
//////////////////////////////////////////////////
// A Simple Ping Client to Test Cluster Simulator
//

#if 0
int main(int argc, char **argv )
{

     if ( argc != 2) {
	fprintf(stdout,"Failed to Start the app\n");
	return -1;
    }
    int rc = atoi(argv[1]);
    cout << "Received from Command line " << rc << endl;
    CPing ps("localhost",7086);
    NODE_PING_INFO  pdu{"localhost", rc};
    if ( !ps.Open() ) {
                 fprintf(stdout, "Failed to Open The Client\n" );
                 return 0;
    }
    while (1) {
           sleep(10);
           PerformLogic();
           ps.SendSignal(&pdu,sizeof(pdu));
    }

    ps.Close();
    return 0;
}

#endif

void *ClickHousePipeline(CProxySocket *ptr, void *lptr);
void *PostgreSQLPipeline(CProxySocket *ptr, void *lptr);
void *MySQLPipeline(CProxySocket *ptr, void *lptr);
void *PassthroughPipeLine(CProtocolSocket *ptr, void *lptr);


static ConfigSingleton &configSingleton = ConfigSingleton::getInstance();

int ch_port = 9100;
//int ch_http_port = 9110;

int main(int argc, char **argv )
{

    if ( argc != 2) {
        fprintf(stdout,"Failed to Start the app\n");
        return -1;
    }
    int rc = atoi(argv[1]);
    rc = (rc > 0 ) ? rc : 9100;
    cout << "Received from Command line " << rc << endl;

    if (rc == 9100 || rc == 9120|| rc == 9110 || rc == 9120) {
        ch_port = rc;
    }

    /*if (rc == 9110 || rc == 9120) {
        ch_http_port = rc;
    }*/



        // Clickhouse Wire level - TLS proxy -- port 9120
    CProxySocket ch(ch_port == 9100 || ch_port == 9120 ? ch_port : 9100);

        // Setting up ClickHouse Proxy
        if(!ch.SetPipeline(ClickHousePipeline))
        {
            cout << "Failed to set ClickHouse Pipeline.................." << endl;
            return -2;

        }

        if (!ch.SetHandler(new CHWirePTHandler ()) ) {
            cout << "Failed to set ClickHouse Handler.................." << endl;
            return -2;
        }

        if (!ch.Start() ) {
            cout << "Failed To Start ClickHouse Proxy Server........................" << endl;
            return -3;

        }



        /*// Clickhouse http proxy with port 9130
        CProxySocket ch_http(ch_http_port == 9110 || ch_http_port == 9120 ? ch_http_port : 9120);

        // Setting Http proxy
        if (!ch_http.SetHandler(new CHttpHandler ()) ) {
            cout << "Failed to set ClickHouse HTTP Handler.................." << endl;
            return -2;
        }

        if(!ch_http.SetPipeline( ClickHousePipeline))
        {
            cout << "Failed to set ClickHouse Passthrough Pipeline.................." << endl;
            return -2;
        }

        if (!ch_http.Start()) {
            cout << "Failed To Start HTTP Proxy Server........................" << endl;
            return -3;
        }*/




        // PostgreSQL proxy with port 9140
        CProxySocket pg(9140);

        // Setting up Postgres Proxy

        if(!pg.SetPipeline(PostgreSQLPipeline))
        {
            cout << "Failed to set PostgreSQL Pipeline.................." << endl;
            return -2;
        }

        if (!pg.SetHandler(new CPostgreSQLHandler ()) ) {
            cout << "Failed to set PostgreSQL Handler.................." << endl;
            return -2;
        }

        if (!pg.Start() ) {
            cout << "Failed To Start PostgreSQL Proxy Server........................" << endl;
            return -3;

        }

        // MySQL proxy with port 9160
        CProxySocket msql(9160);

        // Setting up MySQL Proxy

        if(!msql.SetPipeline(MySQLPipeline))
        {
            cout << "Failed to set MySQL Pipeline.................." << endl;
            return -2;
        }

        if (!msql.SetHandler(new CMySQLHandler ()) ) {
            cout << "Failed to set MySQL Handler.................." << endl;
            return -2;
        }

        if (!msql.Start() ) {
            cout << "Failed To Start MySQL Proxy Server........................" << endl;
            return -3;
        }

    while(1);
    return 0;
}
