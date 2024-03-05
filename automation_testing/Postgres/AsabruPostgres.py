import psycopg2
import yaml
import time
from statistics import mean, median


class AsabruPostgres:
    def __init__(self, config_file='postgres_config.yaml', sql_file='postgres_sql.yaml') -> None:

        with open(config_file, "r") as f:
            config = yaml.safe_load(f)

        with open(sql_file, "r") as f:
            self.sql_statements = yaml.safe_load(f)

        self.host = config['host']
        self.port = config['port']
        self.user = config['username']
        self.password = config['password']
        self.database = config['database']
        self.ca_cert = config['ca_cert']
        self.ssl = True if config['ssl'] == 'True' else False

        if self.ssl:
            print('SSL Enabled!')

    def create_client(self):

        if self.ssl:
            client = psycopg2.connect(database=self.database, user=self.user, password=self.password,
                                      host=self.host, port=self.port, sslmode='verify-ca',
                                      sslrootcert=self.ca_cert)
        else:
            client = psycopg2.connect(database=self.database, user=self.user, password=self.password,
                                      host=self.host, port=self.port)

        return client

    def benchmark_performance(self):
        print('Starting benchmark')
        loop_times = []

        iters = 10
        queries = len(self.sql_statements['read'].keys())
        loop_start = time.time()

        for x in range(iters):

            client = self.create_client()
            iter_start = time.time()

            for query_key in self.sql_statements['read'].keys():
                res = []
                print(self.sql_statements['read'][query_key])
                cursor = client.cursor()
                cursor.execute(self.sql_statements['read'][query_key])
                result = cursor.fetchall()
                for x in result:
                    res.append(x)
                # print(res)
                cursor.close()
            client.close()

            iter_end = time.time() - iter_start

            loop_times.append(iter_end)

        print('Average Time taken : ', mean(loop_times))
        print('Median Time taken : ', median(loop_times))


t = AsabruPostgres()
t.benchmark_performance()
