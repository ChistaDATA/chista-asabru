# Chista Asabru - Load Balancer Strategies

The Chista Asabru proxy has the ability to load balance incoming requests as per the 
load balancing strategy configured. By default, the round-robin strategy is used.

The following configurations showcase the different strategies. The proxy is will reroute
requests based on the load balancer strategy defined, if there are more than 1 `SERVICE`.

## Round Robin
```
<END_POINT name="ch_wire_node1">
    ...
    <loadbalancer-strategy>RoundRobinStrategy</loadbalancer-strategy>
    <SERVICES>
        <SERVICE name="ch_wire_service1">
            <HOST>localhost</HOST>
            <PORT>9111</PORT>
        </SERVICE>
        <SERVICE name="ch_wire_service2">
            <HOST>localhost</HOST>
            <PORT>9112</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```

## Random
```
<END_POINT name="ch_wire_node1">
    ...
    <loadbalancer-strategy>RandomStrategy</loadbalancer-strategy>
    <SERVICES>
        <SERVICE name="ch_wire_service1">
            <HOST>localhost</HOST>
            <PORT>9111</PORT>
        </SERVICE>
        <SERVICE name="ch_wire_service2">
            <HOST>localhost</HOST>
            <PORT>9112</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```

## Weighted Round Robin
```
<END_POINT name="ch_wire_node1">
    ...
    <loadbalancer-strategy>WeightedRoundRobinStrategy</loadbalancer-strategy>
    <SERVICES>
        <SERVICE name="ch_wire_service2" weight="2">
            <HOST>localhost</HOST>
            <PORT>9111</PORT>
        </SERVICE>
        <SERVICE name="ch_wire_service1" weight="1">
            <HOST>localhost</HOST>
            <PORT>9112</PORT>
        </SERVICE>
    </SERVICES>
</END_POINT>
```