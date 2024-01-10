## DotNet Client

This project shows the details about the automation testing of ClickHouse Asabru Proxy using a Microsoft DotNet Client Driver.

The Driver used is Yandex Fully managed ClickHouse Driver from  https://github.com/killwort/ClickHouse-Net

**Note: **An User with privilege to Create a database and a table is required on the ClickHouse server

## Pre-requisites

* Microsoft Dotnet 7
  - Available from https://dotnet.microsoft.com/en-us/download/dotnet/7.0
  
* ClickHouse Driver ClickHouse.Ado Version = 2.0.2.2 for DotNet.
  - This driver is available as a Nuget Package from https://www.nuget.org/packages/ClickHouse.Ado
  
* YamlDotNet - Version = 13.7.1
  - This is a .NET library for YAML file and used for reading configuration files.
  
* Setup and Load Data for testing is described in [here](../README.md)

## Steps
1. Build and run the Asabru server
2. Update the ch_config.yaml with appropriate port numbers, ssl mode and CA certificate path configured in ClickHouse Server
3. Build and Run project using the command

build 
```
dotnet build 
```
run
```
dotnet run 
```
