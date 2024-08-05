#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>
#include <grpcpp/grpcpp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include "proto/messaging.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using messaging::Messenger;
using messaging::MessageRequest;
using messaging::MessageResponse;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;

namespace MyLogger {
    enum VerbosityLevel {
        NONE,
        PEER_ERROR,
        PEER_WARN,
        PEER_INFO,
        PEER_DEBUG
    };

    // Global verbosity level
    VerbosityLevel currentVerbosity = PEER_DEBUG;

    const std::string RESET_COLOR = "\033[0m";
    const std::string RED_COLOR = "\033[31m";
    const std::string YELLOW_COLOR = "\033[33m";
    const std::string GREEN_COLOR = "\033[32m";
    const std::string BLUE_COLOR = "\033[34m";

    void logMessage(VerbosityLevel level, const std::string& message) {
        if (level <= currentVerbosity) {
            switch (level) {
                case PEER_ERROR:
                    std::cerr << RED_COLOR << "ERROR: " << message << RESET_COLOR << std::endl;
                    break;
                case PEER_WARN:
                    std::cerr << YELLOW_COLOR << "WARN: " << message << RESET_COLOR << std::endl;
                    break;
                case PEER_INFO:
                    std::cout << "INFO: " << message << std::endl;
                    break;
                case PEER_DEBUG:
                    std::cout << BLUE_COLOR << "DEBUG: " << message << RESET_COLOR << std::endl;
                    break;
                default:
                    break;
            }
        }
    }
}

class MessengerClient {
public:
    MessengerClient(std::shared_ptr<Channel> channel)
        : stub_(Messenger::NewStub(channel)) {}

    std::string SendMessage(const std::string& message) {
        MessageRequest request;
        request.set_message(message);
        MessageResponse response;
        ClientContext context;
        Status status = stub_->SendMessage(&context, request, &response);

        if (status.ok()) {
            return response.reply();
        } else {
            MyLogger::logMessage(MyLogger::PEER_ERROR, "RPC failed");
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<Messenger::Stub> stub_;
};

class ServerImpl final {
public:
    ServerImpl() {
        node_id_ = generateUniqueId();
        getOwnIpAddress(own_ip_);
        MyLogger::logMessage(MyLogger::PEER_INFO, "Node ID: " + node_id_);
        
    }

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
        MyLogger::logMessage(MyLogger::PEER_INFO, "Server listening on " + server_address);


        // Start peer discovery tasks
        std::thread(&ServerImpl::BroadcastPresence, this).detach();
        std::thread(&ServerImpl::ListenForPeers, this).detach();

        HandleRpcs();
    }

private:
    std::string node_id_;
    char own_ip_[INET_ADDRSTRLEN];
    std::unique_ptr<ServerCompletionQueue> cq_;
    Messenger::AsyncService service_;
    std::unique_ptr<Server> server_;

    std::string generateUniqueId() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < 8; ++i) {
            ss << std::setw(2) << dis(gen);
        }
        return ss.str();
    }

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

    void BroadcastPresence() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        int broadcast = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

        struct sockaddr_in broadcast_addr;
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

        std::string message = "discovery," + node_id_ + ",50051," + std::string(own_ip_);

        while (true) {
            for (int port = 50052; port <= 50062; ++port) {
                broadcast_addr.sin_port = htons(port);
                sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            }
            MyLogger::logMessage(MyLogger::PEER_DEBUG, "Broadcasted message: " + message);

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void ListenForPeers() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        int broadcast = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

        struct sockaddr_in listen_addr;
        memset(&listen_addr, 0, sizeof(listen_addr));
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // Try to bind to ports 50052-50062
        bool bound = false;
        for (int port = 50052; port <= 50062; ++port) {
            listen_addr.sin_port = htons(port);
            if (bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) == 0) {
                MyLogger::logMessage(MyLogger::PEER_INFO, "Successfully bound to port " + std::to_string(port) + " for peer discovery.");

                bound = true;
                break;
            }
        }

        if (!bound) {
            MyLogger::logMessage(MyLogger::PEER_ERROR, "Failed to bind socket to any port in range 50052-50062: " + std::string(strerror(errno)));
            close(sockfd);
            return;
        }

        char buffer[1024];
        struct sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);

        while (true) {
            int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&peer_addr, &addr_len);
            if (len < 0) {
                MyLogger::logMessage(MyLogger::PEER_ERROR, "Failed to receive: " + std::string(strerror(errno)));
                continue;
            }
            buffer[len] = '\0';

            std::string message(buffer);

            MyLogger::logMessage(MyLogger::PEER_DEBUG, "Received message: " + message);

            auto pos1 = message.find(',');
            auto pos2 = message.find(',', pos1 + 1);
            auto pos3 = message.find(',', pos2 + 1);

                    if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
            std::string type = message.substr(0, pos1);
            std::string received_node_id = message.substr(pos1 + 1, pos2 - pos1 - 1);
            int grpc_port = std::stoi(message.substr(pos2 + 1, pos3 - pos2 - 1));
            std::string sender_ip = message.substr(pos3 + 1);

            char peer_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(peer_addr.sin_addr), peer_ip, INET_ADDRSTRLEN);
        
            MyLogger::logMessage(MyLogger::PEER_DEBUG, "Received message: " + message);

            std::string own_message = "discovery," + node_id_ + ",50051," + std::string(own_ip_);
            if (type == "discovery" && message != own_message) {
                MyLogger::logMessage(MyLogger::PEER_INFO, "Discovered peer: " + received_node_id + " at port " + std::to_string(grpc_port) + " with IP " + peer_ip);
                // Create a gRPC client and send a message
                std::string peer_address = std::string(peer_ip) + ":" + std::to_string(grpc_port);
                MessengerClient client(grpc::CreateChannel(peer_address, grpc::InsecureChannelCredentials()));
                MyLogger::logMessage(MyLogger::PEER_INFO, "Sending message to peer: Hello from " + node_id_ + " to " + received_node_id);
                std::string response = client.SendMessage("Hello from " + node_id_ + " to " + received_node_id);
                MyLogger::logMessage(MyLogger::PEER_INFO, "Received response from peer: " + response);
            }
        }
    }
}

void getOwnIpAddress(char* ip) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    if (connect(sock, (const struct sockaddr*)&serv, sizeof(serv)) != 0) {
        MyLogger::logMessage(MyLogger::PEER_ERROR, "Connect failed: " + std::string(strerror(errno)));
        strcpy(ip, "127.0.0.1");
        return;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr*)&name, &namelen) != 0) {
        MyLogger::logMessage(MyLogger::PEER_ERROR, "Connect failed: " + std::string(strerror(errno)));
        strcpy(ip, "127.0.0.1");
    } else {
        inet_ntop(AF_INET, &name.sin_addr, ip, INET_ADDRSTRLEN);
    }

    close(sock);
}
};

int main(int argc, char** argv) {
ServerImpl server;
server.Run();
return 0;
}