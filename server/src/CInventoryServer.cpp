#include "CInventoryServer.h"
#include "TStreamingGenerator.h"
#include <algorithm>
#include <stack>
#include <fstream>
#include <filesystem>

static const char* c_szNotFound = "Item does not exist";

//! @name Utils
//! @{

//! Reads the path of the database file
static auto getDatabaseFilePath()
{
    char szExePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, szExePath, sizeof(szExePath));

    return std::filesystem::path(szExePath).replace_extension("bin");
}

//! Functor to search an item by ID
template <typename T>
class CFindById
{
private:
    uint32_t const m_id;

public:
    //! Creates a comparator to the given ID
    CFindById(uint32_t id)
        : m_id(id)
    {

    }

    //! Checks if the supplied ID matches the desired
    bool operator()(const T& item) const
    {
        return item.id() == m_id;
    }
};

//! Generates a stream of item data
class CItemSubscriber : public TStreamingGenerator<ItemData>
{
private:
    SSubscriberList& m_subscribers;
    std::deque<ItemData> m_items;

public:
    template<typename It>
    CItemSubscriber(SSubscriberList& subscribers, It begin, It end)
        : m_subscribers(subscribers)
        , m_items(begin, end)
    {
        Next();
    }

    void Next() override
    {
        // Send all the initial items first, then wait for new items
        if (m_items.empty())
        {
            m_items.push_back(m_subscribers.get_future().get());
        }

        // Consume each from the front & send
        m_response = m_items.front();
        m_items.pop_front();

        StartWrite(&m_response);
    }
};

//! @}

//! @name SSubscriberList
//! @{
void SSubscriberList::notify_all(const ItemData& item)
{
    std::lock_guard<std::mutex> lock(m_mxSubscribers);
    while (!m_subscribers.empty())
    {
        m_subscribers.front().set_value(item);
        m_subscribers.pop_front();
    }
}

//! Gets a future which will be notified when changes occur
std::future<ItemData> SSubscriberList::get_future()
{
    std::lock_guard<std::mutex> lock(m_mxSubscribers);
    m_subscribers.emplace_back();
    return m_subscribers.back().get_future();
}
//! @}

//! @name CInventoryServer
//! @{
CInventoryServer::CInventoryServer(const std::string& sChannel)
{
    // Load the database
    std::ifstream fsRead;
    const auto filePath = getDatabaseFilePath();

    if (std::filesystem::exists(filePath))
        fsRead.open(filePath);

    if (fsRead.is_open() && m_dataBase.ParseFromIstream(&fsRead))
        std::cout << "Loaded DB from: " << filePath << std::endl;

    // Build the server
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

void CInventoryServer::Save()
{
    const auto filePath = getDatabaseFilePath();
    std::ofstream fsWrite(filePath, std::ios::trunc);
    
    m_dataBase.SerializeToOstream(&fsWrite);
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
    auto* pNewItem = m_dataBase.add_items();
    pNewItem->set_id(uId);

    m_subscribers.notify_all(*pNewItem);

    // Commit
    Save();

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
    auto itFind = std::find_if(pItems->begin(), pItems->end(),
        CFindById<ItemData>(request->delete_id()));

    if (itFind == pItems->end())
    {
        pReactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, c_szNotFound));
    }
    else
    {
        itFind->clear_name();
        itFind->clear_quantity();
        itFind->clear_price();

        m_subscribers.notify_all(*itFind);

        // Perform the deletion
        pItems->erase(itFind, pItems->end());
        pReactor->Finish(grpc::Status::OK);

        // Commit
        Save();

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
        CFindById<ItemData>(request->id()));

    if (itFind == pItems->end())
    {
        pReactor->Finish(grpc::Status(grpc::StatusCode::NOT_FOUND, c_szNotFound));
    }
    else
    {
        std::cout << "[Update] ID: " << request->id() << std::endl;

        // Update only the items that exist in the request
        if (request->has_name())
        {
            itFind->set_name(request->name());
            std::cout << "[Update] Name = " << request->name() << std::endl;
        }

        if (request->has_price())
        {
            itFind->set_price(request->price());
            std::cout << "[Update] Price = " << request->price() << std::endl;
        }

        if (request->has_quantity())
        {
            itFind->set_quantity(request->quantity());
            std::cout << "[Update] Quant = " << request->quantity() << std::endl;
        }

        m_subscribers.notify_all(*itFind);

        pReactor->Finish(grpc::Status::OK);

        // Commit
        Save();
    }

    return pReactor;
}

::grpc::ServerWriteReactor<::ItemData>* CInventoryServer::Subscribe(::grpc::CallbackServerContext* context, const ::Empty* request)
{
    return new CItemSubscriber(m_subscribers, m_dataBase.items().cbegin(), m_dataBase.items().cend());
}
//! @}
