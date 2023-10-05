#ifndef _CINVENTORYCLIENT_H_
#define _CINVENTORYCLIENT_H_

#include <InventoryAPI.pb.h>
#include <InventoryAPI.grpc.pb.h>
#include <grpcpp/grpcpp.h>

//! @brief Client of the inventory
class CInventoryClient
{
private:
    //! The communication channel for this interface
    std::shared_ptr<grpc::Channel> m_pChannel;

    //! The stub if the service
    std::unique_ptr<InventoryService::Stub> m_pStub;

public:
    CInventoryClient(const std::string& sChannel);
    ~CInventoryClient();

    grpc::Status Create(uint32_t& uNewId);
    grpc::Status Delete(uint32_t uId);
    grpc::Status Modify(uint32_t uId, const std::string& sNewName, double dNewPrice);
    grpc::Status Update(uint32_t uId, double dDeltaQuant);
};

#endif