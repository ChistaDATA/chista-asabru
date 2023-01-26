import pandas as pd
from clickhouse_driver import Client
import numpy as np
import os
import yaml
import requests
import lzma
import time
from statistics import mean, median


class AsabruCH:
    def __init__(self, config_file='ch_config.yaml', sql_file = 'ch_sql.yaml') -> None:
        
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

        self.client = Client(host=self.ch_host, user=self.ch_user, password=self.ch_password)
        

    def load_dataset(self):

        databases = self.client.execute('SHOW DATABASES')
        databases = [ database[0] for database in databases]
        
        if self.database_name not in databases:

            self.client.execute(f'CREATE DATABASE {self.database_name}')
            print('Database {self.database_name} created')

        ## download data
        full_url = self.dataset_url + self.dataset_file_name

        # with requests.get(full_url, stream=True) as r:
        #     with open(self.dataset_file_name , 'wb') as f:
        #         f.write(r.content)
        # print(f'Downloaded {self.dataset_file_name}')

        # with lzma.open(self.dataset_file_name) as f, open(self.dataset_csv_name, 'wb') as fout:
        #     file_content = f.read()
        #     fout.write(file_content)
        # print(f'Extracted {self.dataset_csv_name}')

        ## create table

        self.client.execute(self.sql_statements['create']['c1'])
        print('Table "cell_towers"  created')

        ## Load data

        os.system(f'clickhouse-client --host {self.ch_host} --user {self.ch_user} --password {self.ch_password} --query "INSERT INTO {self.database_name}.cell_towers FORMAT CSVWithNames" < cell_towers.csv')
        print('Successfully loaded the data in to the table')

        ## Remove downloaded files

        # if os.path.exists(self.dataset_file_name):
        #     os.remove(self.dataset_file_name)
        #     print('Removed ', self.dataset_file_name)
        # if os.path.exists(self.dataset_csv_name):
        #     os.remove(self.dataset_csv_name)
        #     print('Removed ', self.dataset_csv_name)


    def benchmark_performance(self):
        loop_start = time.time()
        loop_times = []

        iters = 10
        queries = len(self.sql_statements['read'].keys())

        for x in range(iters):

            iter_start = time.time()

            for query_key in self.sql_statements['read'].keys():
                print(self.sql_statements['read'][query_key])
                self.client.execute(self.sql_statements['read'][query_key])
            
            iter_end = time.time() - iter_start

            loop_times.append(iter_end)

        print ('Average Time taken : ', mean(loop_times))
        print ('Median Time taken : ', median(loop_times))


        

