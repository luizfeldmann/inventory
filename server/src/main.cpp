#include <iostream>
#include <csignal>
#include <cxxopts.hpp>
#include "CInventoryServer.h"

//! Flag indicating a stop signal was handled
volatile std::sig_atomic_t g_sigStop;

//! Handler invoked when the installed std::signal is received
static void signalHandler(int signum)
{
    g_sigStop = signum;
}

//! Entry-point for the server
int main(int argc, const char* const* argv)
{
    bool bHelp = false;

    // Connection info
    std::string sHost = "0.0.0.0";
    uint16_t uPort = 8080;

    // Define the command line
    cxxopts::Options cOps(
        "inventory_server",
        "Server of store inventory service");

    cOps.add_options()
        ("h,help", "Show help/usage", cxxopts::value<>(bHelp))
        ("s,server", "Hostname or IP of the server", cxxopts::value<>(sHost))
        ("p,port", "Port number of the server", cxxopts::value<>(uPort))
        ;

    // Parse the command line
    cxxopts::ParseResult cParsed;

    try
    {
        cParsed = cOps.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::parsing& ex)
    {
        // Print error
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    // Print help
    if (bHelp)
    {
        std::cout << cOps.help() << std::endl;
        return 0;
    }

    // Start server
    CInventoryServer cServer(sHost + ":" + std::to_string(uPort));

    std::signal(SIGINT, signalHandler);

    while(!g_sigStop)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        cServer.OnRun();
    }

    return 0;
}
