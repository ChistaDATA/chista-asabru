import snowflake.connector
import yaml
import time
from statistics import mean, median


class AsabruSnowflake:
    def __init__(self, config_file='./snowflake_config.yaml', sql_file = './snowflake_sql.yaml') -> None:

        with open(config_file, "r") as f:
            config = yaml.safe_load(f)

        with open(sql_file, "r") as f:
            self.sql_statements = yaml.safe_load(f)

        self.snowflake_hostname = config['snowflake_hostname']
        self.snowflake_port = config['snowflake_port']
        self.snowflake_account = config['snowflake_account']
        self.snowflake_user = config['snowflake_user']
        self.snowflake_password = config['snowflake_password']
        self.snowflake_warehouse = config['snowflake_warehouse']
        self.snowflake_database = config['snowflake_database']
        self.snowflake_schema = config['snowflake_schema']


    def create_client(self):
        # Establish Snowflake connection
        conn = snowflake.connector.connect(
            user=self.snowflake_user,
            password=self.snowflake_password,
            account=self.snowflake_account,
            warehouse=self.snowflake_warehouse,
            database=self.snowflake_database,
            schema=self.snowflake_schema,
            host=self.snowflake_hostname,
            port=self.snowflake_port,
            insecure_mode=True
        )
        return conn

    def benchmark_performance(self):
        loop_start = time.time()
        loop_times = []

        iters = 10
        queries = len(self.sql_statements['read'].keys())

        print('Starting the benchmark')
        client = self.create_client()
        # Create a cursor object
        cur = client.cursor()

        cur.execute("USE WAREHOUSE " + self.snowflake_warehouse)
        cur.execute("USE DATABASE " + self.snowflake_database)
        cur.execute("USE SCHEMA " + self.snowflake_schema)

        # Execute a SELECT query
        # cur.execute("SELECT * FROM car_prices limit 10")
        #
        # # Fetch the results
        # rows = cur.fetchall()
        #
        # # Print the results
        # print("Query Results:")
        # for row in rows:
        #     print(row)
        for x in range(iters):

            print(f'Iteration {x+1}')

            iter_start = time.time()

            for query_key in self.sql_statements['read'].keys():
                query = self.sql_statements['read'][query_key]
                cur.execute(query)
                print(query)
                time.sleep(1)

            iter_end = time.time() - iter_start
            # client.disconnect()

            loop_times.append(iter_end)

        print ('Average Time taken : ', mean(loop_times))
        print ('Median Time taken : ', median(loop_times))
