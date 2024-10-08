#include "main.h"
#include "logger/Logger.h"
#include "util/ThreadPoolSingleton.h"

/**
 * Database Proxy main function
 * @arg port number
 */
int main(int argc, char **argv) {
    // initialize srand with a seed
    srand(static_cast<unsigned>(time(nullptr)));

    // install our error handler
    signal(SIGSEGV, errorHandler);
    /* ignore SIGPIPE so that server can continue running when client pipe closes abruptly */
    signal(SIGPIPE, SIG_IGN);

    int returnValue = initProxyServers();
    if (returnValue < 0) {
        LOG_ERROR("Error occurred during initializing proxy servers!");
        exit(1);
    }

    returnValue = initProtocolServers();
    if (returnValue < 0) {
        LOG_ERROR("Error occurred during initializing protocol servers!");
        exit(1);
    }

	returnValue = initApiGatewayServers();
	if (returnValue < 0) {
		LOG_ERROR("Error occurred during initializing api gateway server!");
		exit(1);
	}

    pause();
    return 0;
}

/**
 * Error handler
 */
void errorHandler(int sig) {
	void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    // size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);

    exit(1);
}



