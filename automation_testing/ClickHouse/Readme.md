# Asabru ClickHouse

Asabru is a proxy developed in house by ChistaDATA Inc. It supports ClickHouse, MySQL and Postgres. 

This module has the automation test code to validate the functionality and perform load test and performance testing on the Asabru server. For ClickHouse, the code can automatically download the dataset (recipes dataset) load the data and run queries defined in the ch_sql.yaml file. The time taken is measured and the performance can be calculated and compared across.

**Note: **An User with privilege to Create a database and a table is required on the ClickHouse server

## Pre-requisites
* Python3.9+
	- clickhouse-driver
	- requests

## Steps
1. Build and run the Asabru server
2. Update the ch_config.yaml with appropriate port numbers, ssl mode and CA certificate path configured in ClickHouse Server
3. Run the script using the command
> 	$ python3 run_ch_test.py
