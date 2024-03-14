# Chista Asabru - Collecting Query Metrics

## Test Setup

### Download Dataset

Download a sample dataset from this [link](https://www.kaggle.com/datasets/midhundarvin/car-prices/download?datasetVersionNumber=1).

### Create Tables

Create the table in Clickhouse
```
CREATE TABLE car_prices (
    year UInt16,
    make String,
    model String,
    trim String,
    body String,
    transmission String,
    vin String,
    state String,
    condition UInt8,
    odometer UInt32,
    color String,
    interior String,
    seller String,
    mmr UInt32,
    sellingprice UInt32,
    saledate DateTime
) ENGINE = MergeTree()
ORDER BY (vin);
```
### Insert Data

Insert data into clickhouse.
```
clickhouse client --host localhost --port 9100 --user default --query "INSERT INTO default.car_prices FORMAT CSVWithNames" < car_prices.csv
```
### Add Configuration for Tests
Edit `ch_config_2.yaml` with the appropriate values for the proxy and clickhouse details.

### Run Tests
```
cd ${PWD}/automation_testing/ClickHouse/clickhouse-client
sudo chmod u+x run_ch_test.sh
./run_ch_test.sh
```
