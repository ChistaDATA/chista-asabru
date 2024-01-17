# ChistaData Database Proxy - chista-asabru

Asabru is a high-performance SQL Proxy developed by ChistaDATA, designed to improve the scalability and availability of database servers like ClickHouse, PostgreSQL, and MySQL. Asabru server enables users to easily configure and manage their database connections while enhancing their database performance.

At present, Asabru supports TCP/IP, HTTP/HTTPS, and TLS/SSL protocols for ClickHouse. We are also working on adding TLS/SSL support for MySQL and PostgreSQL, while wire-level protocol is already supported.

ChistaDATA Asabru Proxy is available in two forms

- as part of ChistaDATA DBAAS Portal
- as an  independently deployable proxy

### Staring Proxy along with DBAAS
At present, the ChistaDATA Asabru Proxy is seamlessly connected to the ChistaDATA DBAAS portal.

The deployment of the proxy can be initiated during the cluster creation process by choosing the "Deploy Proxy" option on the cluster creation screen.In this case proxy will be using the default port number. Example proxy will be available at the port # 9000 for ClickHouse Wire-level Protocol Proxying

### Independently deployable proxy

For the independent proxying mode , the details about the proxy ports , please see the Documentation section below.

### Documentation

- [Official Documentation](DOCUMENTATION.md)

### Other

https://solid-adventure-73een37.pages.github.io/#/

Generate Documentation Website using : 
https://docsify.js.org/#/quickstart

## Contributing
- [Developer Documentation](DEVELOPER.md)