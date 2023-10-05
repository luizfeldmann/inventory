#include "CInventoryServer.h"
#include "TStreamingGenerator.h"
#include <algorithm>

static const char* c_szNotFound = "Item does not exist";

//! @name 
//! @{
template <typename T>
struct CFindById
{
    uint32_t m_id;

    CFindById(uint32_t id)
        : m_id(id)
    {

    }

    bool operator()(const T& item) const
    {
        return item.id() == m_id;
    }
};
//! @} 

CInventoryServer::CInventoryServer(const std::string& sChannel)
{
    grpc::ServerBuilder cBuilder;

    cBuilder.AddListeningPort(sChannel,
        grpc::InsecureServerCredentials());

    cBuilder.RegisterService(this);

    m_pQueue = cBuilder.AddCompletionQueue();
    m_pServer = cBuilder.BuildAndStart();
}

CInventoryServer::~CInventoryServer()
{
    m_pServer->Shutdown();
    m_pQueue->Shutdown();
}

void CInventoryServer::OnRun()
{
    m_mainThread.poll();
}

::grpc::ServerUnaryReactor* CInventoryServer::CreateItem(::grpc::CallbackServerContext* context, const ::Empty* request, ::NewItemResponse* response)
{
    ENSURE_THREAD(m_mainThread, &CInventoryServer::CreateItem, context, request, response);

    auto* pReactor = context->DefaultReactor();
    
    // Find new ID
    unsigned int uId = 0;

    if (!m_dataBase.items().empty())
        uId = 1 + m_dataBase.items()[m_dataBase.items_size() - 1].id();

    // Set new ID to response
    response->set_new_id(uId);
    pReactor->Finish(grpc::Status::OK);

    // Actually create the item
    m_dataBase.add_items()->set_id(uId);

    // Log
    std::cout << "New item: ID = " << uId << std::endl;

    return pReactor;
}

::grpc::ServerUnaryReactor* CInventoryServer::DeleteItem(::grpc::CallbackServerContext* context, const ::DeleteItemRequest* request, ::Empty* response)
{
    ENSURE_THREAD(m_mainThread, &CInventoryServer::DeleteItem, context, request, response);

    auto* pReactor = context->DefaultReactor();

    // Find item to delete
    auto* pItems = m_dataBase.mutable_items();
    auto itFind = std::find_if(pItems->cbegin(), pItems->cend(),
        CFindById<DatabaseRow>(request->delete_id()));

    if (itFind == pItems->cend())
    {
        pReactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, c_szNotFound));
    }
    else
    {
        pItems->erase(itFind, pItems->end());
        pReactor->Finish(grpc::Status::OK);

        // Log
        std::cout << "Deleted item: ID = " << request->delete_id() << std::endl;
    }
    
    return pReactor;
}

::grpc::ServerUnaryReactor* CInventoryServer::ModifyItem(::grpc::CallbackServerContext* context, const ::ItemData* request, ::Empty* response)
{
    ENSURE_THREAD(m_mainThread, &CInventoryServer::ModifyItem, context, request, response);

    auto* pReactor = context->DefaultReactor();

    // Find item to modify
    auto* pItems = m_dataBase.mutable_items();
    auto itFind = std::find_if(pItems->begin(), pItems->end(), 
        CFindById<DatabaseRow>(request->id()));

    if (itFind == pItems->end())
    {
        pReactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, c_szNotFound));
    }
    else
    {
        itFind->set_name(request->name());
        itFind->set_price(request->price());
        pReactor->Finish(grpc::Status::OK);

        // Log
        std::cout << "Modify item: ID    = " << request->id() << std::endl
                  << "             Name  = " << request->name() << std::endl
                  << "             Price = " << request->price() << std::endl
                  << std::endl;
    }

    return pReactor;
}

::grpc::ServerUnaryReactor* CInventoryServer::ModifyQuantities(::grpc::CallbackServerContext* context, const ::QuantityData* request, ::Empty* response)
{
    ENSURE_THREAD(m_mainThread, &CInventoryServer::ModifyQuantities, context, request, response);

    auto* pReactor = context->DefaultReactor();

    // Find item to modify
    auto* pItems = m_dataBase.mutable_items();
    auto itFind = std::find_if(pItems->begin(), pItems->end(),
        CFindById<DatabaseRow>(request->id()));

    if (itFind == pItems->end())
    {
        pReactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, c_szNotFound));
    }
    else
    {
        itFind->set_quantity(itFind->quantity() + request->quantity());
        pReactor->Finish(grpc::Status::OK);

        // Log
        std::cout << "Modify quant: ID = " << itFind->id() << std::endl 
                  << "           Quant = " << itFind->quantity() << std::endl;
    }

    return pReactor;
}

::grpc::ServerWriteReactor<::ItemData>* CInventoryServer::SubscribeItems(::grpc::CallbackServerContext* context, const ::Empty* request)
{
    return nullptr;
}

::grpc::ServerWriteReactor<::QuantityData>* CInventoryServer::SubscribeQuantities(::grpc::CallbackServerContext* context, const ::Empty* request)
{
    return nullptr;
}