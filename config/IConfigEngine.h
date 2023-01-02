#pragma once
#include "config_types.h"

class IConfigEngine 
{
public:
	virtual  SERVICE Resolve(RESOLVE_CONFIG config) = 0;
};
