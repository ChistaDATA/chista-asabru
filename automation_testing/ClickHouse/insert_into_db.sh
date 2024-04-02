#!/bin/bash

# Load configuration from YAML file
config_file="./ch_config_2.yaml"
sql_file="./ch_sql_2.yaml"

ch_host=$(yq e .host "$config_file")
ch_user=$(yq e .username "$config_file")
ch_password=$(yq e .password "$config_file")
dataset_url=$(yq e .dataset_url "$config_file")
dataset_file_name=$(yq e .dataset_file_name "$config_file")
dataset_csv_name=$(yq e .dataset_csv_name "$config_file")
database_name=$(yq e .database "$config_file")
table_name=$(yq e .table "$config_file")
port=$(yq e .port "$config_file")
ssl=$(yq e .ssl "$config_file")

if [[ "$ssl" == "True" ]]; then
  echo "Using SSL/TLS"
fi

create_client() {
  if [[ "$ssl" == "True" ]]; then
    client_command="clickhouse client --host=$ch_host --port=$port --user=$ch_user --accept-invalid-certificate --secure"
  else
    client_command="clickhouse client --host=$ch_host --port=$port --user=$ch_user "
  fi

  if [[ "$ch_password" != "" ]]; then
    client_command="$client_command --password=$ch_password "
  fi
  echo "$client_command"
}

create_table() {
  client=$(create_client)
  query=$(yq e ".create.c1" "$sql_file")
  $client --query="$query"
}

insert_into_table() {
  client=$(create_client)
  $client --query "INSERT INTO $database_name.$table_name FORMAT CSVWithNames" < $dataset_csv_name
}

create_table
insert_into_table