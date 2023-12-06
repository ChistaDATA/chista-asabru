#include <cstdlib>
#include <string>
#include "TypeFactory.h"
#include "BaseHandler.h"
#include "BaseComputationCommand.h"

template<> BaseHandler *TypeFactory::GetType<BaseHandler>(const std::string type)
{
    if (commands_map.find(type) == commands_map.end())
        loadPlugin<BaseHandler>(type);
    return (BaseHandler *) commands_map[type];
}

template<> BaseComputationCommand *TypeFactory::GetType<BaseComputationCommand>(const std::string type)
{
    if (commands_map.find(type) == commands_map.end())
        loadPlugin<BaseComputationCommand>(type);
    return (BaseComputationCommand *) commands_map[type];
}