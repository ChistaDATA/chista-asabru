#pragma once
#include <string>
#include <unordered_map>
#include "globothy.h"
#include "libnameothy.h"
#include <set>
#include <dlfcn.h>
#include <algorithm>

class TypeFactory
{
private:
    std::unordered_map<std::string, void *> commands_map;
    std::set<std::string> libNames;

public:
    TypeFactory()
    {
        updateLibs();
        for (std::string plugin : libNames)
        {

        }
    };

    void updateLibs() {
        if (!std::getenv("PLUGINS_FOLDER_PATH")) {
            throw std::runtime_error("PLUGINS_FOLDER_PATH environment variable is missing");
        }
        std::cout << "Checking for new libraries :" << std::endl;
        std::string pluginsFolderPath = std::getenv("PLUGINS_FOLDER_PATH");

#if __APPLE__
        const std::string libGlob(pluginsFolderPath + "/*.dylib");
#else
        const std::string libGlob(pluginsFolderPath + "/*.so");
#endif

        std::vector<std::string> filenames = globothy(libGlob);

        size_t before = libNames.size();

        for (std::string plugin : filenames)
        {
            libNames.insert(plugin);
        }
        size_t after = libNames.size();

        if (after - before > 0)
        {
            std::cout << "Found " << after - before << " new plugins";
            std::cout << std::endl;
        }
    }

    /**
     * Loads the plugin into the program space
     */
    template<class T>
    void loadPlugin(std::string pluginName) {
        auto it = std::find_if(libNames.begin(), libNames.end(), [pluginName](const std::string& s) {
            return (s.find(pluginName) != std::string::npos);
        });
        if (it == libNames.end()) {
            throw std::runtime_error("Plugin is not available!");
        }
        std::string plugin = *it;
        try
        {
            void *dlhandle = dlopen(plugin.c_str(), RTLD_LAZY);

            if (dlhandle == NULL)
            {
                printf("Error: %s\n", dlerror());
                exit(1);
            }

            std::pair<std::string, std::string> delibbed =
                    libnameothy(plugin);

            T *(*create)();
            void (*destroy)(T *);

            std::string cn = "create" + delibbed.second;
            std::string dn = "destroy" + delibbed.second;

            create = (T * (*)()) dlsym(dlhandle, cn.c_str());
            destroy = (void (*)(T *))dlsym(dlhandle, dn.c_str());

            T *a = create();
            commands_map[delibbed.second] = a;
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            std::cout << "Error when trying to create dynamic handler instance" << std::endl;
        }
    }
    /**
     * Fetches the base plugin
     * @param type
     * @return
     */
    template<class T>
    T *GetType(std::string type);
};
