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
#include <chrono>
#include <mutex>
#include <map>
#include "proto/messaging.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using messaging::Messenger;
using messaging::MessageRequest;
using messaging::MessageResponse;
using messaging::HeartbeatRequest;
using messaging::HeartbeatResponse;
using messaging::TopologyUpdateRequest;
using messaging::TopologyUpdateResponse;
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


struct PeerInfo {
    std::string node_id;
    std::string ip_address;
    int grpc_port;
    std::chrono::system_clock::time_point last_seen;
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<Messenger::Stub> stub;
};

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
            MyLogger::logMessage(MyLogger::PEER_ERROR, "RPC failed: " + status.error_message());
            return "RPC failed";
        }
    }

    bool Heartbeat() {
        HeartbeatRequest request;
        HeartbeatResponse response;
        ClientContext context;
        Status status = stub_->Heartbeat(&context, request, &response);
        return status.ok();
    }

    bool UpdateTopology(const std::string& topology) {
        TopologyUpdateRequest request;
        request.set_topology(topology);
        TopologyUpdateResponse response;
        ClientContext context;
        Status status = stub_->UpdateTopology(&context, request, &response);
        return status.ok();
    }

    std::string GetNetworkTopology() {
        messaging::NetworkTopologyRequest request;
        messaging::NetworkTopologyResponse response;
        ClientContext context;
        Status status = stub_->GetNetworkTopology(&context, request, &response);
        if (status.ok()) {
            return response.topology();
        } else {
            MyLogger::logMessage(MyLogger::PEER_ERROR, "GetNetworkTopology RPC failed: " + status.error_message());
            return "";
        }
    }

    bool GetHeartbeat() {
        HeartbeatRequest request;
        HeartbeatResponse response;
        ClientContext context;
        
        // Add a 5-second timeout
        std::chrono::system_clock::time_point deadline = 
            std::chrono::system_clock::now() + std::chrono::seconds(5);
        context.set_deadline(deadline);
        
        Status status = stub_->GetHeartbeat(&context, request, &response);
        if (status.ok()) {
            return response.alive();
        } else {
            MyLogger::logMessage(MyLogger::PEER_ERROR, "GetHeartbeat RPC failed: " + status.error_message());
            return false;
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

        // Start peer discovery and management tasks
        std::thread(&ServerImpl::BroadcastPresence, this).detach();
        std::thread(&ServerImpl::ListenForPeers, this).detach();
        std::thread(&ServerImpl::ManagePeerConnections, this).detach();
        std::thread(&ServerImpl::GossipTopology, this).detach();

        HandleRpcs();
    }

private:
    std::string node_id_;
    char own_ip_[INET_ADDRSTRLEN];
    std::unique_ptr<ServerCompletionQueue> cq_;
    Messenger::AsyncService service_;
    std::unique_ptr<Server> server_;
    std::map<std::string, PeerInfo> peers_;
    std::mutex peers_mutex_;

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
        enum class RequestType {
            UNKNOWN,
            SEND_MESSAGE,
            GET_NETWORK_TOPOLOGY,
            GET_HEARTBEAT
        };

        CallData(ServerImpl* server, Messenger::AsyncService* service, ServerCompletionQueue* cq)
            : server_(server), service_(service), cq_(cq),
              message_responder_(&ctx_), topology_responder_(&ctx_), heartbeat_responder_(&ctx_),
              status_(CREATE), request_type_(RequestType::UNKNOWN) {
            Proceed();
        }

        ServerAsyncResponseWriter<messaging::MessageResponse>& GetMessageResponder() {
            return message_responder_;
        }

        ServerAsyncResponseWriter<messaging::NetworkTopologyResponse>& GetTopologyResponder() {
            return topology_responder_;
        }

        ServerAsyncResponseWriter<messaging::HeartbeatResponse>& GetHeartbeatResponder() {
            return heartbeat_responder_;
        }

void Proceed() {
    if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestSendMessage(&ctx_, &message_request_, &GetMessageResponder(), cq_, cq_, this);
        service_->RequestGetNetworkTopology(&ctx_, &topology_request_, &GetTopologyResponder(), cq_, cq_, this);
        service_->RequestGetHeartbeat(&ctx_, &heartbeat_request_, &GetHeartbeatResponder(), cq_, cq_, this);
    } else if (status_ == PROCESS) {
        new CallData(server_, service_, cq_);

        if (ctx_.IsCancelled()) {
            status_ = FINISH;
        } else if (request_type_ == RequestType::SEND_MESSAGE) {
            HandleSendMessage();
        } else if (request_type_ == RequestType::GET_NETWORK_TOPOLOGY) {
            HandleGetNetworkTopology();
        } else if (request_type_ == RequestType::GET_HEARTBEAT) {
            HandleGetHeartbeat();
        }
    } else {
        GPR_ASSERT(status_ == FINISH);
        delete this;
    }
}

        void SetRequestType(RequestType type) {
            request_type_ = type;
        }

    private:
        void HandleSendMessage() {
            message_reply_.set_reply("Server received: " + message_request_.message());
            status_ = FINISH;
            message_responder_.Finish(message_reply_, Status::OK, this);
        }

        void HandleGetNetworkTopology() {
            topology_reply_.set_topology(server_->GetNetworkTopology());
            status_ = FINISH;
            topology_responder_.Finish(topology_reply_, Status::OK, this);
        }

        void HandleGetHeartbeat() {
            MyLogger::logMessage(MyLogger::PEER_DEBUG, "Handling GetHeartbeat request");
            heartbeat_reply_.set_alive(true);
            status_ = FINISH;
            heartbeat_responder_.Finish(heartbeat_reply_, Status::OK, this);
        }

        ServerImpl* server_;
        Messenger::AsyncService* service_;
        ServerCompletionQueue* cq_;
        ServerContext ctx_;

        messaging::MessageRequest message_request_;
        messaging::MessageResponse message_reply_;
        ServerAsyncResponseWriter<messaging::MessageResponse> message_responder_;

        messaging::NetworkTopologyRequest topology_request_;
        messaging::NetworkTopologyResponse topology_reply_;
        ServerAsyncResponseWriter<messaging::NetworkTopologyResponse> topology_responder_;

        messaging::HeartbeatRequest heartbeat_request_;
        messaging::HeartbeatResponse heartbeat_reply_;
        ServerAsyncResponseWriter<messaging::HeartbeatResponse> heartbeat_responder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
        RequestType request_type_;
    };

void HandleRpcs() {
    new CallData(this, &service_, cq_.get());
    void* tag;
    bool ok;
    while (true) {
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        CallData* call_data = static_cast<CallData*>(tag);
        
        if (dynamic_cast<ServerAsyncResponseWriter<messaging::HeartbeatResponse>*>(&call_data->GetHeartbeatResponder())) {
            call_data->SetRequestType(CallData::RequestType::GET_HEARTBEAT);
        } else if (dynamic_cast<ServerAsyncResponseWriter<messaging::NetworkTopologyResponse>*>(&call_data->GetTopologyResponder())) {
            call_data->SetRequestType(CallData::RequestType::GET_NETWORK_TOPOLOGY);
        } else if (dynamic_cast<ServerAsyncResponseWriter<messaging::MessageResponse>*>(&call_data->GetMessageResponder())) {
            call_data->SetRequestType(CallData::RequestType::SEND_MESSAGE);
        }
        
        call_data->Proceed();
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

            std::this_thread::sleep_for(std::chrono::seconds(10));
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

                std::string own_message = "discovery," + node_id_ + ",50051," + std::string(own_ip_);
                if (type == "discovery" && message != own_message) {
                    AddOrUpdatePeer(received_node_id, peer_ip, grpc_port);
                }
            }
        }
    }

    void AddOrUpdatePeer(const std::string& peer_id, const std::string& peer_ip, int peer_port) {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        std::string peer_address = peer_ip + ":" + std::to_string(peer_port);
        
        if (peers_.find(peer_id) == peers_.end()) {
            PeerInfo new_peer;
            new_peer.node_id = peer_id;
            new_peer.ip_address = peer_ip;
            new_peer.grpc_port = peer_port;
            new_peer.last_seen = std::chrono::system_clock::now();
            new_peer.channel = grpc::CreateChannel(peer_address, grpc::InsecureChannelCredentials());
            new_peer.stub = Messenger::NewStub(new_peer.channel);
            peers_[peer_id] = std::move(new_peer);
            MyLogger::logMessage(MyLogger::PEER_INFO, "Added new peer: " + peer_id + " at " + peer_address);
        } else {
            peers_[peer_id].last_seen = std::chrono::system_clock::now();
            MyLogger::logMessage(MyLogger::PEER_INFO, "Updated existing peer: " + peer_id + " at " + peer_address);
        }
    }

    void ManagePeerConnections() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            std::lock_guard<std::mutex> lock(peers_mutex_);
            auto now = std::chrono::system_clock::now();
            for (auto it = peers_.begin(); it != peers_.end();) {
                if (now - it->second.last_seen > std::chrono::minutes(2)) {
                    MessengerClient client(it->second.channel);
                    if (client.Heartbeat()) {
                        it->second.last_seen = now;
                        ++it;
                    } else {
                        MyLogger::logMessage(MyLogger::PEER_WARN, "Peer " + it->first + " is unresponsive. Removing.");
                        it = peers_.erase(it);
                    }
                } else {
                    ++it;
                }
            }
        }
    }

    void GossipTopology() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(60));
            std::string topology = GetNetworkTopology();
            std::lock_guard<std::mutex> lock(peers_mutex_);
            for (const auto& peer : peers_) {
                MessengerClient client(peer.second.channel);
                if (!client.UpdateTopology(topology)) {
                    MyLogger::logMessage(MyLogger::PEER_WARN, "Failed to update topology for peer: " + peer.first);
                }
            }
        }
    }

    std::string GetNetworkTopology() {
        std::stringstream ss;
        std::lock_guard<std::mutex> lock(peers_mutex_);
        ss << node_id_ << "," << own_ip_ << ",50051;";
        for (const auto& peer : peers_) {
            ss << peer.second.node_id << "," << peer.second.ip_address << "," << peer.second.grpc_port << ";";
        }
        return ss.str();
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