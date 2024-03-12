import snowflake.connector

# Snowflake connection parameters
snowflake_user = ''
snowflake_password = ''
snowflake_account = ''

snowflake_warehouse = ''
snowflake_database = ''
snowflake_schema = ''

# Modify the hostname and port
# snowflake_hostname = ''
# snowflake_port = ''  # default HTTPS port

snowflake_hostname = 'root.localhost'
snowflake_port = '9110'  # default HTTPS port

# Establish Snowflake connection
conn = snowflake.connector.connect(
    user=snowflake_user,
    password=snowflake_password,
    account=snowflake_account,
    warehouse=snowflake_warehouse,
    database=snowflake_database,
    schema=snowflake_schema,
    host=snowflake_hostname,
    port=snowflake_port,
    insecure_mode=True
)

try:
    # Create a cursor object
    cur = conn.cursor()
    
    cur.execute("USE WAREHOUSE " + snowflake_warehouse)
    cur.execute("USE DATABASE " + snowflake_database)
    cur.execute("USE SCHEMA " + snowflake_schema)

    # Execute a SELECT query
    cur.execute("SELECT * FROM customer limit 10")

    # Fetch the results
    rows = cur.fetchall()

    # Print the results
    print("Query Results:")
    for row in rows:
        print(row)

finally:
    # Close the cursor and connection
    cur.close()
    conn.close()
