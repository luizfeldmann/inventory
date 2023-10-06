#ifndef _CINVENTORYSERVER_H_
#define _CINVENTORYSERVER_H_

#include <ensurethread.hpp>
#include <grpcpp/grpcpp.h>
#include <InventoryAPI.pb.h>
#include <InventoryAPI.grpc.pb.h>

//! Manages a collection of subscribers
struct SSubscriberList
{
private:
    std::mutex m_mxSubscribers;
    std::deque<std::promise<ItemData>> m_subscribers;

public:
    //! Notify observers of some change
    void notify_all(const ItemData& item);

    //! Gets a future which will be notified when changes occur
    std::future<ItemData> get_future();
};

//! Server wrapper for the service
class CInventoryServer : InventoryService::CallbackService
{
private:
    //! Server handling the RPC calls
    std::unique_ptr<grpc::Server> m_pServer;

    //! Event queue of the server
    std::shared_ptr<grpc::ServerCompletionQueue> m_pQueue;

    //! Ensures execution on the main thread
    CEnsureThread m_mainThread;

    //! The full database of items
    DataBase m_dataBase;

    //! Collection of observers
    SSubscriberList m_subscribers;

    //! @brief Saves the database to disk
    void Save();

    //! @name Implementation of generated API
    //! @{
    
    ::grpc::ServerUnaryReactor* CreateItem(::grpc::CallbackServerContext* context, const ::Empty* request, ::NewItemResponse* response) override;
     
    ::grpc::ServerUnaryReactor* DeleteItem(::grpc::CallbackServerContext* context, const ::DeleteItemRequest* request, ::Empty* response) override;

    ::grpc::ServerUnaryReactor* ModifyItem(::grpc::CallbackServerContext* context, const ::ItemData* request, ::Empty* response) override;

    ::grpc::ServerWriteReactor<::ItemData>* Subscribe(::grpc::CallbackServerContext* context, const ::Empty* request) override;

    //! @}

public:
    //! Constructs associated to a given IP:PORT channel
    CInventoryServer(const std::string& sChannel);
    ~CInventoryServer();

    //! Periodically run the server tasks
    void OnRun();
};

#endif