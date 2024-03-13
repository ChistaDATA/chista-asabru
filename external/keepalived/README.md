<!-- README.md for keepalived setup -->

# Keepalived

When proxying mission-critical applications, such as databases, it's crucial for the proxy to maintain high availability. In Linux environments, this can be achieved through Keepalived. Keepalived employs the Virtual Router Redundancy Protocol (VRRP) to manage failover and load balancing effectively.

## Operational Workflow

1.Keepalived Setup: We configure Keepalived with one server acting as the MASTER and another (or others) as the BACKUP, assigning a Virtual IP (VIP) to the MASTER server.

2.Health Monitoring: The MASTER server continuously monitors the health of the proxied service, broadcasting its status to ensure all nodes are aware of the current service state.

3.Failover Mechanism: The BACKUP server(s) listen for these status broadcasts. In the event of a failure where the MASTER can no longer serve requests, a BACKUP server assumes the role by adopting the VIP, thus directing incoming traffic to itself to ensure uninterrupted service availability.

## Prerequisites

docker
docker-compose

## Setup

1. Clone the repository:

```bash
git clone
```

2. Navigate to the keepalived directory:

```bash
cd keepalived
```

3. Build keepalived_custom image:

```bash
docker build -t keepalived_custom .
```

4. Start the keepalived service:

```bash
docker-compose up -d
```