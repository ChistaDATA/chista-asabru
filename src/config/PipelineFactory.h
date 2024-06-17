#pragma once
#include <string>
#include <unordered_map>

#include "interface/CProtocolSocket.h"
#include "interface/CProxySocket.h"
#include "interface/LibuvProxySocket.h"
#include "socket/Socket.h"
#include <dlfcn.h>
#include <set>
#include "TypeFactory.h"
#include <type_traits>

typedef std::map<std::string, PipelineFunction<CProtocolSocket>> ProtocolPipelineFunctionMap;
typedef std::map<std::string, PipelineFunction<CProxySocket>> ProxyPipelineFunctionMap;
typedef std::map<std::string, PipelineFunction<LibuvProxySocket>> LibuvProxyPipelineFunctionMap;

class PipelineFactory
{
private:
    ProtocolPipelineFunctionMap protocolPipelineFunctionMap;
    ProxyPipelineFunctionMap proxyPipelineFunctionMap;
    LibuvProxyPipelineFunctionMap libuvProxyPipelineFunctionMap;
	std::set<std::string> libNames;
public:
    PipelineFactory()
    {
		updateLibs();
    };

	void updateLibs();

	template <typename T> PipelineFunction<T> GetPipeline(const std::string& pipelineName) {
		if constexpr (std::is_same_v<T, CProxySocket>) {
			// Get Proxy Pipeline
			return proxyPipelineFunctionMap[pipelineName];
		} else if constexpr (std::is_same_v<T, CProtocolSocket>)  {
			// Get Protocol Pipeline
			return protocolPipelineFunctionMap[pipelineName];
		} else if constexpr (std::is_same_v<T, LibuvProxySocket>)  {
			// Get Libuv Pipeline
			return libuvProxyPipelineFunctionMap[pipelineName];
		}
	}
	/**
	 * Register Proxy Pipeline
	 */
	 template <class T>
	 void registerPipeline(const std::string& pipelineName) {
		 if (std::is_same<T, CProxySocket>::value) {
			 // Load Proxy Pipelines
			 loadPipeline<PipelineFunction<CProxySocket>>(pipelineName, proxyPipelineFunctionMap);
		 } else if (std::is_same<T, CProtocolSocket>::value) {
			 // Load Protocol Pipelines
			 loadPipeline<PipelineFunction<CProtocolSocket>>(pipelineName, protocolPipelineFunctionMap);
		 } else if (std::is_same<T, LibuvProxySocket>::value) {
			 // Load Protocol Pipelines
			 loadPipeline<PipelineFunction<LibuvProxySocket>>(pipelineName, libuvProxyPipelineFunctionMap);
		 }
	 }
	/**
	 * Loads the plugin into the program space
	 */
	template <class T> void loadPipeline(const std::string& pluginName, std::map<std::string, T> &functionMap) {
		auto it = std::find_if(libNames.begin(), libNames.end(), [pluginName](const std::string &s) {
			std::string pluginLibFile = s;
			std::pair<std::string, std::string> delibbed = libnameothy(pluginLibFile);
			// pluginName should match the library name
			// for a lib X, the library file name will be of the form <path>/libX.<ext>
			// 'libnameothy' strips the path, 'lib' and extension from the library path
			return pluginName == delibbed.first;
		});
		if (it == libNames.end()) {
			throw std::runtime_error("Plugin is not available : " + pluginName);
		}
		std::string plugin = *it;
		try {
			std::pair<std::string, std::string> delibbed = libnameothy(plugin);
			if (functionMap[delibbed.second] != nullptr) {
				// Command already loaded
				return;
			}

			void *dlhandle = dlopen(plugin.c_str(), RTLD_LAZY);

			if (dlhandle == nullptr) {
				printf("Error: %s\n", dlerror());
				exit(1);
			}

			T pipelineFunc;

			std::string cn = delibbed.second;
			pipelineFunc = (T) dlsym(dlhandle, cn.c_str());
			if (pipelineFunc == nullptr) {
				printf("Error: %s\n", dlerror());
				exit(1);
			}

			functionMap[delibbed.second] = pipelineFunc;

		} catch (std::exception &e) {
			LOG_ERROR(e.what());
			LOG_ERROR("Error when trying to create dynamic pipeline instance");
		}
	}
};
