#include "CInventoryClient.h"

CInventoryClient::CInventoryClient(const std::string& sChannel)
    : m_pChannel(grpc::CreateChannel(sChannel, grpc::InsecureChannelCredentials()))
    , m_pStub(InventoryService::NewStub(m_pChannel))
{
}

CInventoryClient::~CInventoryClient()
{

}

grpc::Status CInventoryClient::Create(uint32_t& uNewId)
{
    grpc::ClientContext Ctx;

    Empty request;
    NewItemResponse response;

    grpc::Status Status = m_pStub->CreateItem(&Ctx, request, &response);
    uNewId = response.new_id();

    return Status;
}

grpc::Status CInventoryClient::Delete(uint32_t uId)
{
    grpc::ClientContext Ctx;

    DeleteItemRequest request;
    request.set_delete_id(uId);

    Empty response;

    return m_pStub->DeleteItem(&Ctx, request, &response);
}

grpc::Status CInventoryClient::Modify(uint32_t uId, const std::string& sNewName, double dNewPrice)
{
    grpc::ClientContext Ctx;

    ItemData request;
    request.set_id(uId);
    request.set_name(sNewName);
    request.set_price(dNewPrice);

    Empty response;

    return m_pStub->ModifyItem(&Ctx, request, &response);
}

grpc::Status CInventoryClient::Update(uint32_t uId, double dDeltaQuant)
{
    grpc::ClientContext Ctx;

    QuantityData request;
    request.set_id(uId);
    request.set_quantity(dDeltaQuant);

    Empty response;

    return m_pStub->ModifyQuantities(&Ctx, request, &response);
}