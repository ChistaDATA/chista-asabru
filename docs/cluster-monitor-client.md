
# Chista Asabru Zookeeper Integration

A ClickHouse cluster is composed of multiple nodes, and the ChistaDATA Asabru proxy is employed to route requests through these nodes. The proxy dynamically updates its node list to accommodate additions or removals of nodes from the cluster. To facilitate this dynamic configuration update, utilities known as Cluster Monitor and Cluster Client are employed.

### Cluster Monitor:

This is an independent program which run and actively listens to a Zookeeper Server (ensemble). When a new cluster is added or removed, the monitor notifies the Asabru Proxy and updates its configuration accordingly. The program monitors cluster nodes by utilising watches associated with each node.

### Cluster Client:

This program creates an Ephemeral node in the Zookeeper server whenever a new node is added to the cluster. Ephemeral nodes are automatically deleted if the corresponding cluster node is removed or crashes. The cluster monitor detects such changes and notifies the proxy, prompting the removal of the deleted node from the proxy. This ensures that further requests are not directed to the deleted nodes.

Using the above mechanism proxy can dynamically update the nodes through which the data has to be routed.

## zk-cluster-monitor

This Python program, developed by ChistaData, serves as a versatile cluster monitor and client for a ZooKeeper cluster.

### Configuration variables used

Following configuration variables are used for both the cluster monitor and client programs

- ZK_HOST - The external ZooKeeper server and its port are specified by the ZK_HOST environment variable.
- MODE - This variable defines the mode in which the program runs and which can be set to either 'monitor' or 'client'.

The program run in Cluster Monitoring Mode when MODE = 'monitor' and it will monitor the cluster nodes for any change like add or removal of nodes.

The program run in Client Mode when MODE ='client' and it will create the ephemeral zookeeper nodes.Zookeeper nodes are added in the Zookeeper ensemble from the dataplane side when clusters nodes are created.

- ZK_ROOT_NODE - The root ZooKeeper node, on which the program listens for changes, is provided through the ZK_ROOT_NODE environment variable.


### Node Monitoring:

In monitor mode, the program continuously watches for changes in the ZooKeeper cluster using the Kazoo library. Upon the addition or removal of a node, the program triggers a callback function that notifies the Asabru proxy at a specified API endpoint.

This ensures that the proxy's services list is dynamically updated based on cluster operations.

Cluster Client Mode (MODE ='client'):

The nodes to be created can be given in a json file nodes.json.

here is a sample

```  
{  
 "/cluster1": "", 
 "/cluster1/ch_http_node1": "",
 "/cluster1/ch_http_node1/ch_http_service2": "localhost:2023"
 }  
```  

ZooKeeper Connection Details:
The program connects to the specified ZooKeeper server using the ZK_HOST environment variable.

### Docker Build
```
docker build -t zk-monitor-client .
```

### Docker Run for Monitor mode
```
docker run --name zk-monitor --network host -e MODE=monitor -e ZK_HOST="host.docker.internal:2181" -e ZK_ROOT_NODE="/cluster1" zk-monitor-client
```

### Docker Run for Client mode
```
docker run --name zk-client --network host -e MODE=client -e ZK_HOST="host.docker.internal:2181" zk-monitor-client
```