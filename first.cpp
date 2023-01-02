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

//#include "CPingClient.h"

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

int main(int argc, char **argv )
{

    if ( argc != 2) {
        fprintf(stdout,"Failed to Start the app\n");
        return -1;
    }
    int rc = atoi(argv[1]);
    rc = (rc < 0 ) ? rc : 9100;
    cout << "Received from Command line " << rc << endl;
    CProxySocket ch(9100);  //default is 9100;
    CProxySocket pg(9200);
    CProxySocket msql(9300);

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

    if (!ch.Start() ) {
        cout << "Failed To Start ClickHouse Proxy Server........................" << endl;
        return -3;

    }

    if (!pg.Start() ) {
        cout << "Failed To Start PostgreSQL Proxy Server........................" << endl;
        return -3;

    }

    if (!msql.Start() ) {
        cout << "Failed To Start MySQL Proxy Server........................" << endl;
        return -3;

    }
    while(1);
    return 0;
}
