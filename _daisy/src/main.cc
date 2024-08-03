#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "proto/messaging.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using messaging::Messenger;
using messaging::MessageRequest;
using messaging::MessageResponse;

class ServerImpl final {
public:
    ~ServerImpl() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run() {
        std::string server_address("0.0.0.0:50051");

        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        
        // Set keep-alive options
        builder.SetOption(grpc::MakeChannelArgumentOption("grpc.keepalive_time_ms", 60000));
        builder.SetOption(grpc::MakeChannelArgumentOption("grpc.keepalive_timeout_ms", 20000));
        builder.SetOption(grpc::MakeChannelArgumentOption("grpc.keepalive_permit_without_calls", 1));
        
        // Enable compression
        builder.SetOption(grpc::MakeChannelArgumentOption("grpc.default_compression_algorithm", GRPC_COMPRESS_GZIP));
        
        // Optimize resource usage
        builder.SetMaxReceiveMessageSize(1024 * 1024 * 10); // 10 MB
        builder.SetMaxSendMessageSize(1024 * 1024 * 10); // 10 MB
        builder.SetOption(grpc::MakeChannelArgumentOption("grpc.so_reuseport", 1));

        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

private:
    class CallData {
    public:
        CallData(Messenger::AsyncService* service, ServerCompletionQueue* cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            Proceed();
        }

        void Proceed() {
            if (status_ == CREATE) {
                status_ = PROCESS;
                service_->RequestSendMessage(&ctx_, &request_, &responder_, cq_, cq_, this);
            } else if (status_ == PROCESS) {
                new CallData(service_, cq_);
                reply_.set_reply("Server received: " + request_.message());
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

    private:
        Messenger::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerContext ctx_;
        MessageRequest request_;
        MessageResponse reply_;
        ServerAsyncResponseWriter<MessageResponse> responder_;
        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

    void HandleRpcs() {
        new CallData(&service_, cq_.get());
        void* tag;
        bool ok;
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<CallData*>(tag)->Proceed();
        }
    }

    std::unique_ptr<ServerCompletionQueue> cq_;
    Messenger::AsyncService service_;
    std::unique_ptr<Server> server_;
};

int main(int argc, char** argv) {
    ServerImpl server;
    server.Run();
    return 0;
}