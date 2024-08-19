#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "proto/messaging.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using heartbeat::AnotherRequest;
using heartbeat::AnotherResponse;
using heartbeat::HeartbeatRequest;
using heartbeat::HeartbeatResponse;
using heartbeat::HeartbeatService;

class ServerImpl final {
   public:
    ~ServerImpl() {
        server_->Shutdown();
        cq_->Shutdown();
    }

    void Run() {
        std::string server_address("0.0.0.0:50051");

        ServerBuilder builder;
        builder.AddListeningPort(server_address,
                                 grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        HandleRpcs();
    }

   private:
    class BaseCallData {
       public:
        virtual void
        Proceed() = 0;  // Pure virtual function, making this an abstract class
        virtual ~BaseCallData() {}  // Virtual destructor
    };
    
    class HeartbeatCallData : public BaseCallData {
       public:
        HeartbeatCallData(HeartbeatService::AsyncService *service,
                          ServerCompletionQueue *cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            Proceed();
        }

        void Proceed() override {
            if (status_ == CREATE) {
                status_ = PROCESS;
                service_->RequestHeartbeat(&ctx_, &request_, &responder_, cq_,
                                           cq_, this);
            } else if (status_ == PROCESS) {
                new HeartbeatCallData(service_, cq_);

                response_.set_status("OK");
                status_ = FINISH;
                responder_.Finish(response_, Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

       private:
        HeartbeatService::AsyncService *service_;
        ServerCompletionQueue *cq_;
        ServerContext ctx_;

        HeartbeatRequest request_;
        HeartbeatResponse response_;
        ServerAsyncResponseWriter<HeartbeatResponse> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

    class AnotherMethodCallData : public BaseCallData {
       public:
        AnotherMethodCallData(HeartbeatService::AsyncService *service,
                              ServerCompletionQueue *cq)
            : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
            Proceed();
        }

        void Proceed() override {
            if (status_ == CREATE) {
                status_ = PROCESS;
                service_->RequestAnotherMethod(&ctx_, &request_, &responder_,
                                               cq_, cq_, this);
            } else if (status_ == PROCESS) {
                new AnotherMethodCallData(service_, cq_);

                response_.set_result("Processed " + request_.data());
                status_ = FINISH;
                responder_.Finish(response_, Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
            }
        }

       private:
        HeartbeatService::AsyncService *service_;
        ServerCompletionQueue *cq_;
        ServerContext ctx_;

        AnotherRequest request_;
        AnotherResponse response_;
        ServerAsyncResponseWriter<AnotherResponse> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

    void HandleRpcs() {
        new HeartbeatCallData(&service_,
                              cq_.get());  // For handling Heartbeat RPC
        new AnotherMethodCallData(&service_,
                                  cq_.get());  // For handling AnotherMethod RPC

        void *tag;
        bool ok;
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);

            BaseCallData *call_data = static_cast<BaseCallData *>(tag);
            call_data->Proceed();
        }
    }

    std::unique_ptr<ServerCompletionQueue> cq_;
    HeartbeatService::AsyncService service_;
    std::unique_ptr<Server> server_;
};

int main(int argc, char **argv) {
    ServerImpl server;
    server.Run();
    return 0;
}