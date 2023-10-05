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
        bModify = false,
        bUpdate = false;

    // Connection info
    std::string sHost = "127.0.0.1";
    uint16_t uPort = 8080;

    uint32_t uId = 0;
    std::string sName;
    double dQuantity = .0;
    double dPrice = .0;

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
        ("modify", "Modifies the data of an item", cxxopts::value<>(bModify))
        ("update", "Updates the quantity of an item", cxxopts::value<>(bUpdate))
        ;

    static const char
        * szParId = "id",
        * szParName = "name",
        * szParQuant = "quant",
        * szParPrice = "price";

    cOps.add_options("Parameters")
        (szParId, "", cxxopts::value<>(uId))
        (szParName, "", cxxopts::value<>(sName))
        (szParQuant, "", cxxopts::value<>(dQuantity))
        (szParPrice, "", cxxopts::value<>(dPrice))
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
    bModify = bModify && !bDelete && (bHasId || bCreate) && bHasName && bHasPrice;
    bUpdate = bUpdate && !bDelete && (bHasId || bCreate) && bHasQuant;

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

        if (bDelete)
            Status = cClient.Delete(uId);

        if (bModify)
            Status = cClient.Modify(uId, sName, dPrice);

        if (bUpdate)
            Status = cClient.Update(uId, dQuantity);

        // Print possible error
        if (!Status.ok())
        {
            std::cerr << Status.error_message() << std::endl;
            return Status.error_code();
        }
    }

    return 0;
}