#!/bin/bash

# Load configuration from YAML file
config_file="../ch_config_2.yaml"
sql_file="../ch_sql_2.yaml"

ch_host=$(yq e .host "$config_file")
ch_user=$(yq e .username "$config_file")
ch_password=$(yq e .password "$config_file")
dataset_url=$(yq e .dataset_url "$config_file")
dataset_file_name=$(yq e .dataset_file_name "$config_file")
dataset_csv_name=$(yq e .dataset_csv_name "$config_file")
database_name=$(yq e .database "$config_file")
table_name=$(yq e .table "$config_file")
port=$(yq e .port "$config_file")
ca_cert=$(yq e .ca_cert "$config_file")
client_config=$(yq e .client_config "$config_file")
ssl=$(yq e .ssl "$config_file")

if [[ "$ssl" == "True" ]]; then
    echo "Using SSL/TLS"
fi

create_client() {
    if [[ "$ssl" == "True" ]]; then
        client_command="clickhouse client --host=$ch_host --port=$port --user=$ch_user --ca_cert=$ca_cert --secure"
    else
        client_command="clickhouse client --host=$ch_host --port=$port --user=$ch_user "
    fi

    if [[ "$ch_password" != "" ]]; then
      client_command="$client_command --password=$ch_password "
    fi
    echo "$client_command"
}

benchmark_performance() {
    loop_start=$(date +%s)
    loop_times=()

    iters=10
    queries=$(yq e '.read | length' "$sql_file")

    echo "Starting the benchmark"

    for (( x=0; x<iters; x++ )); do
        client=$(create_client)
        echo "Iteration $((x+1))"

        iter_start=$(gdate +%s%N)

        for query_key in $(yq e '.read | keys | .[]' "$sql_file"); do
            query=$(yq e ".read.$query_key" "$sql_file")
            $client --query="$query" > /dev/null
            echo "$query"
            sleep 1
        done

        iter_end=$(( $(gdate +%s%N) - iter_start ))
        unset client

        loop_times+=("$(((iter_end/1000000) - 100)) ")
    done

    total_time=0
    for time in "${loop_times[@]}"; do
        total_time=$((total_time + time))
    done

    average_time=$((total_time / iters))
    median_time=$(printf "%s\n" "${loop_times[@]}" | sort -n | awk 'NR==5')

    echo "Average Time taken : $average_time milliseconds"
    echo "Median Time taken : $median_time milliseconds"
}

create_client
benchmark_performance