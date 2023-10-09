#pragma once
#include <string>
#include <unordered_map>
#include "globothy.h"
#include "libnameothy.h"
#include "BaseHandler.h"

#include <set>
#include <dlfcn.h>

class TypeFactory
{
private:
    std::unordered_map<std::string, BaseHandler *> commands_map;
    std::set<std::string> libNames;

public:
    TypeFactory()
    {
        updateLibs();
        for (std::string plugin : libNames)
        {
            try
            {
                std::cout << plugin << std::endl;
                void *dlhandle = dlopen(plugin.c_str(), RTLD_LAZY);

                if (dlhandle == NULL)
                {
                    printf("Error: %s\n", dlerror());
                    exit(1);
                }

                std::pair<std::string, std::string> delibbed =
                    libnameothy(plugin);

                BaseHandler *(*create)();
                void (*destroy)(BaseHandler *);

                std::string cn = "create" + delibbed.second;
                std::string dn = "destroy" + delibbed.second;

                create = (BaseHandler * (*)()) dlsym(dlhandle, cn.c_str());
                destroy = (void (*)(BaseHandler *))dlsym(dlhandle, dn.c_str());

                BaseHandler *a = create();
                commands_map[delibbed.second] = a;
            }
            catch (std::exception &e)
            {
                std::cout << e.what() << std::endl;
                std::cout << "Error when trying to create dynamic handler instance" << std::endl;
            }
        }
    };

    void updateLibs();
    BaseHandler *GetType(std::string type);
};
