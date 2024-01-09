# Setup Database

Connect to clickhouse via default user and create test database and user
```
CREATE DATABASE IF NOT EXISTS asabru_testing;
CREATE USER asabru IDENTIFIED BY '123456';
GRANT ALL ON asabru_testing.* TO asabru;
```

Run the following python scripts
```
python3 download_dataset.py
python3 insert_into_db.py
```
