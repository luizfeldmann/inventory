#include <cxxopts.hpp>
#include <iostream>
#include "CInventoryClient.h"

//! Entry-point for the client
int main(int argc, const char* const* argv)
{
    // Commands
    bool
        bHelp = false,
        bCreate = false,
        bDelete = false,
        bUpdate = false,
        bObserve = false;

    // Connection info
    std::string sHost = "127.0.0.1";
    uint16_t uPort = 8080;

    uint32_t uId = 0;       //!< The ID of the item passed via CLI
    std::string sName;      //!< The name of the item passed via CLI
    float fQuantity = .0;   //!< The quantity of the item passed via CLI
    float fPrice = .0;      //!< The price of the item passed via CLI

    // Define the command line
    cxxopts::Options cOps(
        "inventory_client",
        "Client to access store inventory");

    cOps.add_options("Basic")
        ("h,help", "Show help/usage", cxxopts::value<>(bHelp))
        ("s,server", "Hostname or IP of the server", cxxopts::value<>(sHost))
        ("p,port", "Port number of the server", cxxopts::value<>(uPort))
        ;

    cOps.add_options("Commands")
        ("create", "Creates a new item", cxxopts::value<>(bCreate))
        ("delete", "Deletes an item", cxxopts::value<>(bDelete))
        ("update", "Updates the name, price or quantity of an item", cxxopts::value<>(bUpdate))
        ("observe", "Subscribe to stream of item data", cxxopts::value<>(bObserve))
        ;

    static const char
        * szParId = "id",
        * szParName = "name",
        * szParQuant = "quant",
        * szParPrice = "price";

    cOps.add_options("Parameters")
        (szParId, "", cxxopts::value<>(uId))
        (szParName, "", cxxopts::value<>(sName))
        (szParQuant, "", cxxopts::value<>(fQuantity))
        (szParPrice, "", cxxopts::value<>(fPrice))
        ;

    // Parse the command line
    cxxopts::ParseResult cParsed;

    try
    {
        cParsed = cOps.parse(argc, argv);
    }
    catch (const cxxopts::exceptions::parsing& ex)
    {
        std::cerr << ex.what() << std::endl;
        return -1;
    }

    // Validate the option combinations
    const bool bHasId    = cParsed.count(szParId);
    const bool bHasName  = cParsed.count(szParName);
    const bool bHasQuant = cParsed.count(szParQuant);
    const bool bHasPrice = cParsed.count(szParPrice);

    bCreate = bCreate && !bDelete && !bHasId;
    bDelete = bDelete && !bCreate && bHasId;
    bUpdate = ((bUpdate && bHasId) || (bCreate)) && (bHasQuant || bHasPrice || bHasName);

    // Print help
    if (bHelp)
    {
        std::cout << cOps.help() << std::endl;
        return 0;
    }
    else
    {
        grpc::Status Status;

        // Connect to server
        CInventoryClient cClient(sHost + ":" + std::to_string(uPort));

        if (bCreate)
        {
            Status = cClient.Create(uId);
            std::cout << uId << std::endl;
        }
        else if (bDelete)
        {
            Status = cClient.Delete(uId);
        }

        if (Status.ok() && bUpdate)
        {
            Status = cClient.Update(uId,
                bHasName ? sName.c_str() : nullptr,
                bHasPrice ? &fPrice : nullptr,
                bHasQuant ? &fQuantity : nullptr
            );
        }

        if (bObserve)
            Status = cClient.Observe();

        // Print possible error
        if (!Status.ok())
        {
            std::cerr << Status.error_message() << std::endl;
            return Status.error_code();
        }
    }

    return 0;
}