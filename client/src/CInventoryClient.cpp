#include "CInventoryClient.h"
#include <iomanip>

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

grpc::Status CInventoryClient::Update(uint32_t uId, const char* pNewName, const float* pNewPrice, const float* pNewQuantity)
{
    grpc::ClientContext Ctx;

    ItemData request;
    request.set_id(uId);

    if (pNewName)
        request.set_name(pNewName);

    if (pNewPrice)
        request.set_price(*pNewPrice);

    if (pNewQuantity)
        request.set_quantity(*pNewQuantity);

    Empty response;

    return m_pStub->ModifyItem(&Ctx, request, &response);
}

//! Prints one row of items
static void printRow(const ItemData& item)
{
    std::cout 
        << std::setw(5) << item.id() << "\t"
        << std::setw(10) << (item.has_name() ? item.name() : "") << "\t"
        << std::setw(15) << (item.has_price() ? std::to_string(item.price()) : "-") << "\t"
        << std::setw(15) << (item.has_quantity() ? std::to_string(item.quantity()) : "-") << "\t"
        << std::endl;
}

grpc::Status CInventoryClient::Observe()
{
    grpc::ClientContext Ctx;

    Empty request;
    auto pWriter = m_pStub->Subscribe(&Ctx, request);

    ItemData response;
    while (pWriter->Read(&response))
    {
        printRow(response);
    }

    return pWriter->Finish();
}