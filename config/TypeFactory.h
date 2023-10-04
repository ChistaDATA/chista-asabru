#pragma once
#include <string>
#include <unordered_map>
#include "globothy.h"
#include "libnameothy.h"
#include "CProxyHandler.h"
#include <set>
#include <dlfcn.h>

class TypeFactory
{
private:
    std::unordered_map<std::string, CProxyHandler *> commands_map;
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

                CProxyHandler *(*create)();
                void (*destroy)(CProxyHandler *);

                std::string cn = "create" + delibbed.second;
                std::string dn = "destroy" + delibbed.second;

                create = (CProxyHandler * (*)()) dlsym(dlhandle, cn.c_str());
                destroy = (void (*)(CProxyHandler *))dlsym(dlhandle, dn.c_str());

                CProxyHandler *a = create();
                commands_map[delibbed.second] = a;
            }
            catch (std::exception &e)
            {
                cout << e.what() << endl;
                cout << "Error when trying to create dynamic handler instance" << endl;
            }
        }
    };

    void updateLibs();
    CProxyHandler *GetType(std::string type);
};
