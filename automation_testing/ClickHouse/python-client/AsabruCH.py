from clickhouse_driver import Client
import os
import yaml
import requests
import lzma
import time
from statistics import mean, median


class AsabruCH:
    def __init__(self, config_file='../ch_config.yaml', sql_file = '../ch_sql.yaml') -> None:
        
        with open(config_file, "r") as f:
            config = yaml.safe_load(f)

        with open(sql_file, "r") as f:
            self.sql_statements = yaml.safe_load(f)

        self.ch_host = config['host']
        self.ch_user = config['username']
        self.ch_password = config['password']
        self.dataset_url = config['dataset_url'] 
        self.dataset_file_name = config['dataset_file_name']
        self.dataset_csv_name = config['dataset_csv_name']
        self.database_name = config['database']
        self.table_name = config['table']
        self.port = config['port']
        self.ca_cert = config['ca_cert']
        self.client_config = config['client_config']
        self.ssl = True if config['ssl']=='True' else False

        if self.ssl:
            print('Using SSL/TLS')


    def create_client(self):
        if not self.ssl:
            client = Client(host=self.ch_host, user=self.ch_user, password=self.ch_password, port=self.port, 
                                send_receive_timeout=600)
        if self.ssl:
            client = Client(host=self.ch_host, user=self.ch_user, password=self.ch_password, port=self.port, 
                                send_receive_timeout=600, ca_certs=self.ca_cert, secure=True )

        return client

    def benchmark_performance(self):
        loop_start = time.time()
        loop_times = []

        iters = 10
        queries = len(self.sql_statements['read'].keys())

        print('Starting the benchmark')

        for x in range(iters):
            client = self.create_client()
            print(f'Iteration {x+1}')

            iter_start = time.time()

            for query_key in self.sql_statements['read'].keys():
                query = self.sql_statements['read'][query_key]
                client.execute(query)
                print(query)
            
            iter_end = time.time() - iter_start
            client.disconnect()

            loop_times.append(iter_end)

        print ('Average Time taken : ', mean(loop_times))
        print ('Median Time taken : ', median(loop_times))


        

