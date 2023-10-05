#ifndef _T_STREAMING_GENERATOR_H_
#define _T_STREAMING_GENERATOR_H_

//! @brief Reactor to generate streaming messages
template <typename TResponse>
class TStreamingGenerator : public ::grpc::ServerWriteReactor<TResponse>
{
protected:
    //! Storage for the response data
    TResponse m_response;

    //! Generates the next sample
    virtual void Next() = 0;

public:
    //! Self destructs when ready
    void OnDone() override
    { 
        delete this;
    }

    //! Notification of async write completion
    void OnWriteDone(bool) override
    {
        Next();
    }
};

#endif