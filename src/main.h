#pragma once

// #include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <list>
#include <map>
#include <iostream>
#include <string>
#include <utility>
/*for O_RDONLY*/
#include <fcntl.h>
#include "protocol_server.h"
#include "proxy_server.h"

void errorHandler(int sig);
