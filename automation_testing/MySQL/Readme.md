# Asabru Mysql

Asabru proxy does also support MySQL. This folder has the scripts to test the functionality as well as performance and load.

## MySQL Prerequisites
* classicmodels database loaded - Refer [Link](https://www.mysqltutorial.org/mysql-sample-database.aspx "Database")
* Non root user with privileges to the classicmodels database

## Python prerequisites
* Python 3.9+
* mysql-connector-python

## Steps
* Ensure the dataset is loaded with user created for the test runs
* Build and run the Asabru
* Update the mysql_config.yaml with relevant configuration
* Execute the python program
> $ python3 AsabruMySQL.py 
