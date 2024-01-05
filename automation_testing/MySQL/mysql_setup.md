### Install MySQL CLI
```
brew install mysql-client
```

### Start Docker compose - MySQL
```
docker compose up -d
```

### Connect to MySQL
```
mysql -h 127.0.0.1 -p -u root
```

### Connect via SSL from outside docker
```
mysql -h 127.0.0.1 -p -u asabru --ssl-mode=required 
mysql -u root -p --ssl-mode=required --ssl-ca=/Users/midhundarvin/certs/mysql/root-ca.pem --ssl-cert=/Users/midhundarvin/certs/mysql/client-cert.pem --ssl-key=/Users/midhundarvin/certs/mysql/client-key.pem
```

### Connect via SSL from docker exec
```
mysql -u root -p --ssl-ca=/etc/certs/root-ca.pem --ssl-cert=/etc/certs/client-cert.pem --ssl-key=/etc/certs/client-key.pem
```

### Load Database
```
mysql -h 127.0.0.1 -u root -p < mysqlsampledatabase.sql
```

### Create User
```
CREATE USER 'asabru' IDENTIFIED BY '123456';
```

### Grant Privileges
```
GRANT ALL PRIVILEGES ON *.* TO 'asabru' WITH GRANT OPTION;
FLUSH PRIVILEGES;
```

### Enable SSL only connect for the user (optional)
```
UPDATE mysql.user SET ssl_type = 'ANY' WHERE user = 'asabru';
```