#include "main.h"
#include "Logger.h"
#include "ThreadPoolSingleton.h"

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

    /**
    * Creating via Logger singleton class will start a thread, that
    * listens for log inputs and outputs them asyncronously
    */
    Logger *logger = Logger::getInstance();

    int returnValue = initProxyServers();
    if (returnValue < 0) {
        logger->Log("main", "ERROR", "Error occurred during initializing proxy servers!");
        exit(1);
    }

    returnValue = initProtocolServers();
    if (returnValue < 0) {
        logger->Log("main", "ERROR", "Error occurred during initializing protocol servers!");
        exit(1);
    }

    while (true);
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

    pid_t myPid = getpid();
    std::string pstackCommand = "pstack ";
    std::stringstream ss;
    ss << myPid;
    pstackCommand += ss.str();
    system(pstackCommand.c_str());
    // backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}



