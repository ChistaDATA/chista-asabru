#include "LoadBalancerFactory.h"

LoadBalancerStrategy<RESOLVED_SERVICE> * LoadBalancerFactory::GetLoadBalancerStrategy(const std::string &strategyName) {
    return this->loadBalancerStrategyMap[strategyName];
}
