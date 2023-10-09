#include <cstdlib>
#include <string>
#include "TypeFactory.h"
#include "CProxyHandler.h"

void TypeFactory::updateLibs()
{
    std::cout << "Checking for new libs" << std::endl;
    std::string pluginsFolderPath = std::getenv("PLUGINS_FOLDER_PATH");

#if __APPLE__
    const std::string libGlob(pluginsFolderPath + "/*.dylib");
#else
    const std::string libGlob(pluginsFolderPath + "/*.so");
#endif

    std::vector<std::string> filenames = globothy(libGlob);

    size_t before = libNames.size();
    // libLock.lock(); // get locked, boi ------------------------------//
    for (std::string plugin : filenames)
    {                            //
        libNames.insert(plugin); //
    }                            //
    // libLock.unlock();            // got 'em ------------------------------------//
    size_t after = libNames.size();

    if (after - before > 0)
    {
        std::cout << "Found " << after - before << " new plugins";
        std::cout << std::endl;
    }
}

BaseHandler *TypeFactory::GetType(std::string type)
{
    return commands_map[type];
};