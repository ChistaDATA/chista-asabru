### Install Postgres Lib

**Linux**
```
sudo apt-get install libpq-dev
```
**MacOS**
```
brew install libpq
```

### Connect to Postgres
```
psql -h localhost -p 5432 -U postgres
```

### Connect to Postgres in TLS mode
```
psql "sslmode=require host=localhost dbname=dvdrental" -h localhost -p 5432 -U postgres
```

### Restore Database
```
pg_restore -U postgres -d dvdrental ./dvdrental.tar
```

Note: If you get "ERROR: schema "public" already exists" when restoring, it is a [known
issue](https://community.synopsys.com/s/article/How-to-deal-with-ERROR-schema-public-already-exists-when-running-DB-restoreHow-to-deal-with-ERROR-schema-public-already-exists-when-running-DB-restore), and can be ignored.

### Create User
```
CREATE USER asabru WITH ENCRYPTED PASSWORD '123456';
```
### Grant Privileges
```
\c dvdrental;
GRANT ALL  ON ALL TABLES IN SCHEMA public TO asabru;
```
