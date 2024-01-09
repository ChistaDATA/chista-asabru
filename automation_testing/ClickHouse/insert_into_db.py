from clickhouse_driver import Client
import os
import yaml
import requests
import lzma

config_file = 'ch_config.yaml'
sql_file = 'ch_sql.yaml'


class DatabaseInserter:
    def __init__(self, config_file='ch_config.yaml', sql_file='ch_sql.yaml') -> None:
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
        self.ssl = True if config['ssl'] == 'True' else False

        if self.ssl:
            print('Using SSL/TLS')

    def create_client(self):
        if not self.ssl:
            client = Client(host=self.ch_host, user=self.ch_user, password=self.ch_password, port=self.port,
                            send_receive_timeout=600)
        if self.ssl:
            client = Client(host=self.ch_host, user=self.ch_user, password=self.ch_password, port=self.port,
                            send_receive_timeout=600, ca_certs=self.ca_cert, secure=True)

        return client

    def load_dataset(self):

        client = self.create_client()

        databases = client.execute('SHOW DATABASES')
        databases = [database[0] for database in databases]

        if self.database_name not in databases:
            client.execute(f'CREATE DATABASE {self.database_name}')
            print('Database ' + self.database_name + ' created')

        client.execute(f'DROP TABLE IF EXISTS {self.database_name}.cell_towers')

        ## create table
        client.execute(self.sql_statements['create']['c1'])
        print('Table "cell_towers"  created')

        ## Create User
        client.disconnect()

        ## Load data
        if not self.ssl:
            os.system(
                f'clickhouse client --host {self.ch_host} --user {self.ch_user} --password {self.ch_password} --port {self.port} --query "INSERT INTO {self.database_name}.cell_towers FORMAT CSVWithNames" < cell_towers.csv')
        else:
            # os.system(f'clickhouse client --secure --host {self.ch_host} --user {self.ch_user} --password {self.ch_password} --port {self.port} --query "INSERT INTO {self.database_name}.cell_towers FORMAT CSVWithNames" < cell_towers.csv')
            os.system(
                f'clickhouse client --secure --accept-invalid-certificate --host {self.ch_host} --port {self.port} --user {self.ch_user} --password {self.ch_password} --query "INSERT INTO {self.database_name}.cell_towers FORMAT CSVWithNames" < cell_towers.csv')

        print('Successfully loaded the data in to the table')

        ## Remove downloaded files

        # if os.path.exists(self.dataset_file_name):
        #     os.remove(self.dataset_file_name)
        #     print('Removed ', self.dataset_file_name)
        # if os.path.exists(self.dataset_csv_name):
        #     os.remove(self.dataset_csv_name)
        #     print('Removed ', self.dataset_csv_name)


if __name__ == "__main__":
    databaseInserter = DatabaseInserter()
    databaseInserter.load_dataset()
