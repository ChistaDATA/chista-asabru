# Asabru Postgres
Asabru is a proxy developed by ChistaDATA INc. This module is to test the Asabru server's functionality and performance on Postgres.

This code is meant to be run on Postgres on dvdrental dataset.


## Postgres Pre-requisites
* Postgres 14
* dvdrental dataset [Available here](https://www.postgresqltutorial.com/postgresql-getting-started/postgresql-sample-database/)
* Dedicated postgres user for running the script (e.g CREATE USER asabru WITH ENCRYPTED PASSWORD '123456';
)
*

## Python Pre-requisites
* Python 3.9+
* psycopg2

## Running the Script
* Run the Asabru proxy
* Ensure the dvdrental dataset is loaded and the user has privileges to the database
* Make the necessary changes in the postgres_config.yaml
* Execute the tests by entering  "**python3 AsabruPostgres.py**" in the terminal