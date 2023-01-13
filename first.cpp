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

int main(int argc, char **argv )
{

    if ( argc != 2) {
        fprintf(stdout,"Failed to Start the app\n");
        return -1;
    }
    int rc = atoi(argv[1]);
    rc = (rc > 0 ) ? rc : 9100;
    cout << "Received from Command line " << rc << endl;

    // Clickhouse Wire level -- port 9100
    CProxySocket ch_wire( 9100);
    RESOLVE_CONFIG resolveConfig = {"cluster1", "ch_wire_http_node1", "ch_wire_service1"};
    auto configValues = configSingleton.Resolve(resolveConfig);

        // Setting up ClickHouse Proxy
        if(!ch_wire.SetPipeline(ClickHousePipeline))
        {
            cout << "Failed to set ClickHouse Pipeline.................." << endl;
            return -2;

        }

        if (!ch_wire.SetHandler(new CHWirePTHandler ()) ) {
            cout << "Failed to set ClickHouse Handler.................." << endl;
            return -2;
        }

        if(!ch_wire.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!ch_wire.Start() ) {
            cout << "Failed To Start ClickHouse Proxy Server........................" << endl;
            return -3;

        }

        ////////////////////////////////////////////////////////////////////////////
        // Clickhouse http proxy with port 9110
        //
        CProxySocket ch_http(9110);
        resolveConfig = {"cluster1", "ch_wire_http_node1", "ch_http_service1"};
        configValues = configSingleton.Resolve(resolveConfig);

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

        if(!ch_http.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!ch_http.Start()) {
            cout << "Failed To Start HTTP Proxy Server........................" << endl;
            return -3;
        }
        //////////////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////////////////
        // Clickhouse Wire level TLS -- port 9120
        //
        CProxySocket ch_wire_tls( 9120);
        resolveConfig = {"cluster1", "ch_tls_node1", "ch_tls_wire_service1"};
        configValues = configSingleton.Resolve(resolveConfig);

        // Setting up ClickHouse Proxy
        if(!ch_wire_tls.SetPipeline(ClickHousePipeline))
        {
            cout << "Failed to set ClickHouse Pipeline.................." << endl;
            return -2;

        }

        if (!ch_wire_tls.SetHandler(new CHWirePTHandler ()) )
        {
            cout << "Failed to set ClickHouse Handler.................." << endl;
            return -2;
        }

        if(!ch_wire_tls.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!ch_wire_tls.Start() ) {
            cout << "Failed To Start ClickHouse Proxy Server........................" << endl;
            return -3;

        }
        ////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////
        // Clickhouse http proxy with tls --port 9130
        //
        CProxySocket ch_http_tls(9130);
        resolveConfig = {"cluster1", "ch_tls_node1", "ch_tls_http_service1"};
        configValues = configSingleton.Resolve(resolveConfig);

        // Setting Http proxy
        if (!ch_http_tls.SetHandler(new CHttpHandler ()) ) {
            cout << "Failed to set ClickHouse HTTP Handler.................." << endl;
            return -2;
        }

        if(!ch_http_tls.SetPipeline( ClickHousePipeline))
        {
            cout << "Failed to set ClickHouse Passthrough Pipeline.................." << endl;
            return -2;
        }

        if(!ch_http_tls.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!ch_http_tls.Start()) {
            cout << "Failed To Start HTTP Proxy Server........................" << endl;
            return -3;
        }
        //////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////////////////
        // PostgreSQL proxy with port 9140
        CProxySocket pg(9140);
        resolveConfig = {"cluster2", "pg_node1", "pg_wire_service1"};
        configValues = configSingleton.Resolve(resolveConfig);

        // Setting up Postgres Proxy

        if(!pg.SetPipeline(PostgreSQLPipeline))
        {
            cout << "Failed to set PostgreSQL Pipeline.................." << endl;
            return -2;
        }

        if (!pg.SetHandler(new CPostgreSQLHandler ()) )
        {
            cout << "Failed to set PostgreSQL Handler.................." << endl;
            return -2;
        }
        if(!pg.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!pg.Start() ) {
            cout << "Failed To Start PostgreSQL Proxy Server........................" << endl;
            return -3;

        }


        /////////////////////////////////////////////////////////////////////////////////
        // PostgreSQL proxy with TLS port 9150
        CProxySocket pg_tls(9150);
        resolveConfig = {"cluster2", "pg_node1", "pg_tls_service1"};
        configValues = configSingleton.Resolve(resolveConfig);

        // Setting up Postgres Proxy

        if(!pg_tls.SetPipeline(PostgreSQLPipeline))
        {
            cout << "Failed to set PostgreSQL Pipeline.................." << endl;
            return -2;
        }

        if (!pg_tls.SetHandler(new CPostgreSQLHandler ()) )
        {
            cout << "Failed to set PostgreSQL Handler.................." << endl;
            return -2;
        }
        if(!pg_tls.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!pg_tls.Start() ) {
            cout << "Failed To Start PostgreSQL Proxy Server........................" << endl;
            return -3;

        }

        /////////////////////////////////////////////////////////////////////////////////
        // MySQL proxy with port 9160
        CProxySocket msql(9160);
        resolveConfig = {"cluster2","mysql_node1","mysql_wire_service1"};
        configValues = configSingleton.Resolve(resolveConfig);
        // Setting up MySQL Proxy

        if(!msql.SetPipeline(MySQLPipeline))
        {
            cout << "Failed to set MySQL Pipeline.................." << endl;
            return -2;
        }

        if (!msql.SetHandler(new CMySQLHandler ()) )
        {
            cout << "Failed to set MySQL Handler.................." << endl;
            return -2;
        }

        if(!msql.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!msql.Start() ) {
            cout << "Failed To Start MySQL Proxy Server........................" << endl;
            return -3;
        }


        /////////////////////////////////////////////////////////////////////////////////
        // MySQL proxy with TLS port 9170
        CProxySocket msql_tls(9170);
        resolveConfig = {"cluster2","mysql_node1","mysql_tls_service1"};
        configValues = configSingleton.Resolve(resolveConfig);
        // Setting up MySQL Proxy

        if(!msql_tls.SetPipeline(MySQLPipeline))
        {
            cout << "Failed to set MySQL Pipeline.................." << endl;
            return -2;
        }

        if (!msql_tls.SetHandler(new CMySQLHandler ()) )
        {
            cout << "Failed to set MySQL Handler.................." << endl;
            return -2;
        }

        if(!msql_tls.SetConfigValues(configValues))
        {
            cout << "Failed to set Config values.................." << endl;
            return -2;
        }

        if (!msql_tls.Start() ) {
            cout << "Failed To Start MySQL Proxy Server........................" << endl;
            return -3;
        }

    while(1);
    return 0;
}
