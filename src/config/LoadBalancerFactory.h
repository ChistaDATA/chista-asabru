#pragma once
#include <map>
#include "config/config_types.h"
#include "load_balancer/RoundRobinStrategy.h"
#include "load_balancer/RandomStrategy.h"
#include "load_balancer/WeightedRoundRobinStrategy.h"

typedef std::map<std::string, LoadBalancerStrategy<RESOLVED_SERVICE> *> LoadBalancerStrategyMap;

class LoadBalancerFactory {
private:
    LoadBalancerStrategyMap loadBalancerStrategyMap;
public:
    LoadBalancerFactory() {
        // Create LoadBalancerStrategy Mappings
        loadBalancerStrategyMap["RoundRobinStrategy"] = new RoundRobinStrategy<RESOLVED_SERVICE>();
        loadBalancerStrategyMap["RandomStrategy"] = new RandomStrategy<RESOLVED_SERVICE>();
        loadBalancerStrategyMap["WeightedRoundRobinStrategy"] = new WeightedRoundRobinStrategy<RESOLVED_SERVICE>();
    };

    LoadBalancerStrategy<RESOLVED_SERVICE>* GetLoadBalancerStrategy(const std::string &strategyName);
};
