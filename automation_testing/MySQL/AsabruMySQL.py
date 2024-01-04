import mysql.connector
import os
import yaml
import time
from statistics import mean, median

class AsabruMySQL:
    def __init__(self, config_file='msql_config.yaml', sql_file = 'msql_sql.yaml') -> None:
        
        with open(config_file, "r") as f:
            config = yaml.safe_load(f)

        with open(sql_file, "r") as f:
            self.sql_statements = yaml.safe_load(f)

        self.host = config['host']
        self.user = config['username']
        self.password = config['password']
        self.database = config['database']
        self.port = config['port']
        self.ca_cert = config['ca_cert']
        self.ssl = True if config['ssl']=='True' else False

        if self.ssl:
            print ('SSL Enabled')


    def create_client(self):


        if self.ssl:
            client = mysql.connector.connect(host=self.host,
                                            port=self.port,
                                            user=self.user,
                                            password=self.password,
                                            ssl_ca=self.ca_cert)

        else:
            client = mysql.connector.connect(host=self.host,
                                            port=self.port,
                                            user=self.user,
                                            password=self.password)

        return client


    def benchmark_performance(self):
        print ('Starting benchmark')
        loop_times = []

        iters = 2
        queries = len(self.sql_statements['read'].keys())
        loop_start = time.time()

        for x in range(iters):

            client = self.create_client()

            iter_start = time.time()

            for query_key in self.sql_statements['read'].keys():
                res = []
                print(self.sql_statements['read'][query_key])
                client = self.create_client()
                cursor = client.cursor()
                # cursor.execute("SHOW STATUS LIKE '%Ssl_cipher%'")
                # print(cursor.fetchall())
                cursor.execute(self.sql_statements['read'][query_key])
                result = cursor.fetchall()
                # for x in myresult:
                #     print(x)
                cursor.close()
                client.close()
            
            iter_end = time.time() - iter_start
            # client.close()
            loop_times.append(iter_end)

        print ('Average Time taken : ', mean(loop_times))
        print ('Median Time taken : ', median(loop_times))



t = AsabruMySQL()
t.benchmark_performance()