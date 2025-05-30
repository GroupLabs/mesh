#include <arpa/inet.h>
#include <errno.h>
#include <grpcpp/grpcpp.h>
#include <netdb.h>
#include <unistd.h>

#include <torch/script.h>  // One of the headers needed for torch::jit::load and torch operations


#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <uuid/uuid.h>

#include "include/log.h"

#include "proto/messaging.grpc.pb.h"

#include "model_and_tensor_generated.h"
#include "model_and_tensor.grpc.fb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

using messaging::HeartbeatRequest;
using messaging::HeartbeatResponse;
using messaging::MessageRequest;
using messaging::MessageResponse;
using messaging::Messenger;
using messaging::TopologyUpdateRequest;
using messaging::TopologyUpdateResponse;

// namespace Log {
// enum VerbosityLevel { NONE, PEER_ERROR, PEER_WARN, PEER_INFO, PEER_DEBUG, PEER_DEBUG2 };

// VerbosityLevel currentVerbosity = PEER_DEBUG2;

// const std::string RESET_COLOR = "\033[0m";
// const std::string RED_COLOR = "\033[31m";
// const std::string YELLOW_COLOR = "\033[33m";
// const std::string PURPLE_COLOR = "\033[35m";
// const std::string BLUE_COLOR = "\033[34m";

// void logMessage(VerbosityLevel level, const std::string &message) {
//     if (level <= currentVerbosity) {
//         switch (level) {
//             case PEER_ERROR:
//                 std::cerr << RED_COLOR << "ERROR: " << message << RESET_COLOR
//                           << std::endl;
//                 break;
//             case PEER_WARN:
//                 std::cerr << YELLOW_COLOR << "WARN: " << message << RESET_COLOR
//                           << std::endl;
//                 break;
//             case PEER_INFO:
//                 std::cout << "INFO: " << message << std::endl;
//                 break;
//             case PEER_DEBUG:
//                 std::cout << BLUE_COLOR << "DEBUG: " << message << RESET_COLOR
//                           << std::endl;
//                 break;
//             case PEER_DEBUG2:
//                 std::cout << PURPLE_COLOR << "DEBUG2: " << message << RESET_COLOR
//                             << std::endl;
//                 break;
//             default:
//                 break;
//         }
//     }
// }
// }  // namespace Log

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

    std::string SendMessage(const std::string &message) {
        MessageRequest request;
        request.set_message("Processing: " + message);
        MessageResponse response;
        ClientContext context;
        Status status = stub_->SendMessage(&context, request, &response);

        if (status.ok()) {
            return "response.reply();";
        } else {
            Log::logMessage(Log::PEER_ERROR,
                                 "RPC failed: " + status.error_message());
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

    bool UpdateTopology(const std::string &topology) {
        TopologyUpdateRequest request;
        request.set_topology(topology);
        TopologyUpdateResponse response;
        ClientContext context;
        auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(8000);
        context.set_deadline(deadline);
        Log::logMessage(Log::PEER_DEBUG, "Attempting to update topology: " + topology);
        Status status = stub_->UpdateTopology(&context, request, &response);
        if (!status.ok()) {
            Log::logMessage(Log::PEER_ERROR, "Failed to update topology. Error code: " + std::to_string(status.error_code()) + ", Message: " + status.error_message());
            return false;
        }
        return true;
    }

   private:
    std::unique_ptr<Messenger::Stub> stub_;
};

class ServerImpl final {
   public:
    ServerImpl() {
        node_id_ = generateUniqueId();
        getOwnIpAddress(own_ip_);
        Log::logMessage(Log::PEER_INFO, "Node ID: " + node_id_);
    }

    ~ServerImpl() {
        if (server_) {
            server_->Shutdown();
            void* tag;
            bool ok;
            while (cq_->Next(&tag, &ok)) {
                BaseCallData* call_data = static_cast<BaseCallData*>(tag);
                if (ok) {
                    call_data->Proceed(true);
                } else {
                    delete call_data;
                }
            }
            cq_->Shutdown();
        }
    }

    void Run() {
        std::string server_address("0.0.0.0:50051");

        ServerBuilder builder;
        builder.AddListeningPort(server_address,
                                 grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        // Register the FlatBuffers-based model service
        builder.RegisterService(&model_service_);

        // Set keep-alive options
        builder.SetOption(
            grpc::MakeChannelArgumentOption("grpc.keepalive_time_ms", 60000));
        builder.SetOption(grpc::MakeChannelArgumentOption(
            "grpc.keepalive_timeout_ms", 20000));
        builder.SetOption(grpc::MakeChannelArgumentOption(
            "grpc.keepalive_permit_without_calls", 1));

        // Enable compression
        builder.SetOption(grpc::MakeChannelArgumentOption(
            "grpc.default_compression_algorithm", GRPC_COMPRESS_GZIP));

        // Optimize resource usage
        builder.SetMaxReceiveMessageSize(1024 * 1024 * 10);  // 10 MB
        builder.SetMaxSendMessageSize(1024 * 1024 * 10);     // 10 MB
        builder.SetOption(
            grpc::MakeChannelArgumentOption("grpc.so_reuseport", 1));

        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        Log::logMessage(Log::PEER_INFO,
                             "Server listening on " + server_address);

        // peer discovery and management tasks
        std::thread(&ServerImpl::BroadcastPresence, this).detach();
        std::thread(&ServerImpl::ListenForPeers, this).detach();
        std::thread(&ServerImpl::ManagePeerConnections, this).detach();
        std::thread(&ServerImpl::GossipTopology, this).detach();

        HandleRpcs();
    }

   private:
    std::string generateUniqueId() {
        uuid_t uuid;
        uuid_generate_random(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
    }

    class BaseCallData {
    public:
        virtual void Proceed(bool ok) = 0;
        virtual ~BaseCallData() {}
    };

    class HeartbeatCallData : public BaseCallData {
        public:
            HeartbeatCallData(Messenger::AsyncService *service, ServerCompletionQueue *cq)
                : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
                service_->RequestHeartbeat(&ctx_, &request_, &responder_, cq_, cq_, this);
            }

            void Proceed(bool ok) override {
                if (status_ == CREATE) {
                    Log::logMessage(Log::PEER_DEBUG2, "Heartbeat - CREATE");

                    if (!ok) {
                        // the call did not successfully start
                        delete this;
                        return;
                    }

                    new HeartbeatCallData(service_, cq_); // new handler for the next request

                    status_ = FINISH;

                    // respond immediately
                    response_.set_success(true);
                    responder_.Finish(response_, grpc::Status::OK, this);

                } else if (status_ == FINISH) {
                    Log::logMessage(Log::PEER_DEBUG2, "Heartbeat - FINISH");

                    GPR_ASSERT(status_ == FINISH);
                    delete this; // clean up
                }
            }

        private:
            Messenger::AsyncService *service_;
            ServerCompletionQueue *cq_;
            ServerContext ctx_;

            HeartbeatRequest request_;
            HeartbeatResponse response_;
            ServerAsyncResponseWriter<HeartbeatResponse> responder_;

            enum CallStatus { CREATE, PROCESS, FINISH };
            CallStatus status_;
    };

    class MessageCallData : public BaseCallData {
        public:
            MessageCallData(Messenger::AsyncService *service, ServerCompletionQueue *cq)
                : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
                // Start listening for a new SendMessage request
                service_->RequestSendMessage(&ctx_, &request_, &responder_, cq_, cq_, this);
            }

            void Proceed(bool ok) override {
                if (status_ == CREATE) {
                    Log::logMessage(Log::PEER_DEBUG2, "Message - CREATE");

                    if (!ok) {
                        // the call did not successfully start
                        delete this;
                        return;
                    }

                    new MessageCallData(service_, cq_); // new handler for the next request

                    status_ = FINISH;

                    // respond immediately
                    response_.set_message(request_.message());
                    responder_.Finish(response_, grpc::Status::OK, this);

                } else if (status_ == FINISH) {
                    Log::logMessage(Log::PEER_DEBUG2, "Message - FINISH");

                    GPR_ASSERT(status_ == FINISH);
                    delete this; // clean up
                }
            }

        private:
            Messenger::AsyncService *service_;
            ServerCompletionQueue *cq_;
            ServerContext ctx_;

            MessageRequest request_;
            MessageResponse response_;
            ServerAsyncResponseWriter<MessageResponse> responder_;

            enum CallStatus { CREATE, PROCESS, FINISH };
            CallStatus status_;
    };

    class TopologyUpdateCallData : public BaseCallData {
        public:
            TopologyUpdateCallData(Messenger::AsyncService *service, ServerCompletionQueue *cq)
                : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
                // Start listening for an UpdateTopology request
                service_->RequestUpdateTopology(&ctx_, &request_, &responder_, cq_, cq_, this);
            }

            void Proceed(bool ok) override {
                if (status_ == CREATE) {
                    Log::logMessage(Log::PEER_DEBUG2, "TopologyUpdate - CREATE");

                    if (!ok) {
                        // the call did not successfully start
                        delete this;
                        return;
                    }

                    new TopologyUpdateCallData(service_, cq_); // new handler for the next request

                    status_ = FINISH;

                    // respond immediately
                    response_.set_success(true);
                    responder_.Finish(response_, grpc::Status::OK, this);

                } else if (status_ == FINISH) {
                    Log::logMessage(Log::PEER_DEBUG2, "TopologyUpdate - FINISH");

                    GPR_ASSERT(status_ == FINISH);
                    delete this; // clean up
                }
            }

        private:
            Messenger::AsyncService *service_;
            ServerCompletionQueue *cq_;
            ServerContext ctx_;

            TopologyUpdateRequest request_;
            TopologyUpdateResponse response_;
            ServerAsyncResponseWriter<TopologyUpdateResponse> responder_;

            enum CallStatus { CREATE, PROCESS, FINISH };
            CallStatus status_;
    };

    class ReceiveModelAndTensorCallData : public BaseCallData {
     public:
        ReceiveModelAndTensorCallData(myservice::ModelService::AsyncService* service,
                                      grpc::ServerCompletionQueue* cq)
            : service_(service),
              cq_(cq),
              responder_(&ctx_),
              messages_received_(false),
              status_(CREATE) {
            service_->RequestReceiveModelAndTensor(&ctx_, &responder_, cq_, cq_, this);
        }

        void Proceed(bool ok) override {
            if (!ok && status_ != FINISH) {
                // Handle errors if necessary
            }

            if (status_ == CREATE) {
                Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - CREATE");

                new ReceiveModelAndTensorCallData(service_, cq_); // Prepare for next call

                status_ = READ;
                responder_.Read(&input_msg_, this);

            } else if (status_ == READ) {
                Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - READ");

                if (!ok) {
                    // Handle end of stream or no messages
                    if (!messages_received_) {
                        Log::logMessage(Log::PEER_WARN, "ReceiveModelAndTensor - no messages received");
                        status_ = FINISH;
                        flatbuffers::grpc::Message<myservice::ReceiveModelAndTensorResponse> empty_response;
                        responder_.Finish(empty_response,
                                          grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "No messages received from client"),
                                          this);
                    } else {
                        // Stream ended after receiving at least one message
                        Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - stream ended with >=1 messages");

                        // Run inference here before finishing
                        runInferenceAndLog();

                        status_ = FINISH;
                        flatbuffers::grpc::MessageBuilder builder;
                        auto status_str = builder.CreateString("Success");
                        auto resp_offset = myservice::CreateReceiveModelAndTensorResponse(builder, status_str);
                        builder.Finish(resp_offset);
                        auto response_msg = builder.ReleaseMessage<myservice::ReceiveModelAndTensorResponse>();
                        responder_.Finish(response_msg, grpc::Status::OK, this);
                    }

                } else {
                    // Transition to PROCESS state without issuing another Read
                    Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - READ - message received");
                    status_ = PROCESS;
                    // Do NOT call Read() here
                    // The PROCESS state will handle processing and initiate the next Read
                    Proceed(true); // Manually proceed to PROCESS
                }

            } else if (status_ == PROCESS) {
                Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - PROCESS");

                if (!ok) {
                    // Stream ended after receiving at least one message
                    Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - PROCESS - stream ended");

                    // Run inference here before finishing
                    runInferenceAndLog();

                    status_ = FINISH;
                    flatbuffers::grpc::MessageBuilder builder;
                    auto status_str = builder.CreateString("Success");
                    auto resp_offset = myservice::CreateReceiveModelAndTensorResponse(builder, status_str);
                    builder.Finish(resp_offset);
                    auto response_msg = builder.ReleaseMessage<myservice::ReceiveModelAndTensorResponse>();
                    responder_.Finish(response_msg, grpc::Status::OK, this);

                } else {
                    // Process the message
                    Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - PROCESS - message received");
                    const myservice::InputData* input_data = input_msg_.GetRoot();
                    if (!input_data) {
                        Log::logMessage(Log::PEER_ERROR, "Invalid input data received");
                        status_ = FINISH;
                        flatbuffers::grpc::Message<myservice::ReceiveModelAndTensorResponse> empty_response;
                        responder_.Finish(empty_response,
                                          grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid input data"),
                                          this);
                        return;
                    }

                    messages_received_ = true;

                    // Check if it's a FileChunk or TensorChunk
                    if (input_data->file()) {
                        size_t file_size = input_data->file()->data()->size();
                        Log::logMessage(Log::PEER_DEBUG2, "Received file_chunk with size: " + std::to_string(file_size));
                        file_data_.insert(file_data_.end(),
                                          input_data->file()->data()->begin(),
                                          input_data->file()->data()->end());
                    }

                    if (input_data->tensor()) {
                        size_t tensor_size = input_data->tensor()->data()->size();
                        Log::logMessage(Log::PEER_DEBUG2, "Received tensor_chunk with size: " + std::to_string(tensor_size));
                        tensor_data_.insert(tensor_data_.end(),
                                            input_data->tensor()->data()->begin(),
                                            input_data->tensor()->data()->end());
                    }

                    // After processing, transition back to READ and issue the next Read
                    status_ = READ;
                    responder_.Read(&input_msg_, this);
                }

            } else if (status_ == FINISH) {
                Log::logMessage(Log::PEER_DEBUG2, "ReceiveModelAndTensor - FINISH");
                delete this;
            }
        }

     private:

        void runInferenceAndLog() {
            // 1. Write received model data to a temporary file.
            std::string tmp_model_path = "/tmp/received_model.pt";
            {
                std::ofstream ofs(tmp_model_path, std::ios::binary);
                if (!ofs) {
                    Log::logMessage(Log::PEER_ERROR, "Failed to open temp model file for writing");
                    return;
                }
                ofs.write(reinterpret_cast<const char*>(file_data_.data()), file_data_.size());
            }

            std::ifstream ifs(tmp_model_path, std::ios::binary | std::ios::ate);
            auto actual_size = ifs.tellg();
            if (actual_size <= 0) {
                Log::logMessage(Log::PEER_ERROR, "Model file size is zero or unreadable");
            }

            std::cout << "File size written: " << actual_size << std::endl;

            // 2. Load the model
            torch::jit::script::Module model;
            try {
                model = torch::jit::load(tmp_model_path);
                model.eval();
            } catch (const c10::Error& e) {
                Log::logMessage(Log::PEER_ERROR, "Error loading model: " + std::string(e.what()));
                return;
            }

            std::vector<int64_t> input_shape = {3, 3, 3, 3};
            if ((int64_t)tensor_data_.size() != (3*3*3*3)) {
                Log::logMessage(Log::PEER_WARN, "Tensor data size does not match expected input shape");
                return;
            }

            torch::Tensor input_tensor = torch::from_blob(tensor_data_.data(), input_shape, torch::kFloat);
            // Move to CPU for simplicity; if you have GPU, you can use `input_tensor.to(torch::kCUDA)` if model is on CUDA.
            input_tensor = input_tensor.clone(); // Make it own the data if needed.

            // 4. Run inference
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(input_tensor);

            torch::Tensor output;
            try {
                auto result = model.forward(inputs);
                output = result.toTensor();
            } catch (const c10::Error& e) {
                Log::logMessage(Log::PEER_ERROR, "Error running inference: " + std::string(e.what()));
                return;
            }

            // 5. Log the inference results
            // For example, if output is a single-dimensional tensor:
            Log::logMessage(Log::PEER_INFO, "Model output: " + tensorToString(output));
        }

        std::string tensorToString(const torch::Tensor& tensor) {
            // Just a helper to convert a small tensor into a string for logging.
            // For large tensors, consider summarizing instead of printing all values.
            std::ostringstream oss;
            oss << tensor;
            return oss.str();
        }

        myservice::ModelService::AsyncService* service_;
        grpc::ServerCompletionQueue* cq_;
        grpc::ServerContext ctx_;

        grpc::ServerAsyncReader<flatbuffers::grpc::Message<myservice::ReceiveModelAndTensorResponse>,
                                flatbuffers::grpc::Message<myservice::InputData>> responder_;

        flatbuffers::grpc::Message<myservice::InputData> input_msg_;
        std::vector<uint8_t> file_data_;
        std::vector<float> tensor_data_;
        bool messages_received_;

        enum CallStatus { CREATE, READ, PROCESS, FINISH };
        CallStatus status_;
    };

    void HandleRpcs() {
        // protobuf rpcs
        new HeartbeatCallData(&service_, cq_.get());
        new MessageCallData(&service_, cq_.get());
        new TopologyUpdateCallData(&service_, cq_.get());

        // flatbuffer rpcs
        new ReceiveModelAndTensorCallData(&model_service_, cq_.get());

        void *tag;
        bool ok;
        while (true) {
            if (!cq_->Next(&tag, &ok)) {
                break;
            }
            BaseCallData *call_data = static_cast<BaseCallData *>(tag);
            call_data->Proceed(ok);
        }
    }

    void BroadcastPresence() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            Log::logMessage(Log::PEER_ERROR, "Failed to create socket: " + std::string(strerror(errno)));
            return;
        }

        int broadcast = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
            Log::logMessage(Log::PEER_ERROR, "Failed to set socket options: " + std::string(strerror(errno)));
            close(sockfd);
            return;
        }

        struct sockaddr_in broadcast_addr;
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

        std::string message =
            "discovery," + node_id_ + ",50051," + std::string(own_ip_);

        while (true) {
            for (int port = 50052; port <= 50062; ++port) {
                broadcast_addr.sin_port = htons(port);
                if (sendto(sockfd, message.c_str(), message.size(), 0,
                           (struct sockaddr *)&broadcast_addr,
                           sizeof(broadcast_addr)) < 0) {
                    Log::logMessage(Log::PEER_ERROR, "Failed to broadcast message: " + std::string(strerror(errno)));
                }
            }
            Log::logMessage(Log::PEER_DEBUG,
                                 "Broadcasted message: " + message);

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

    void ListenForPeers() {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            Log::logMessage(Log::PEER_ERROR, "Failed to create socket: " + std::string(strerror(errno)));
            return;
        }

        int broadcast = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
            Log::logMessage(Log::PEER_ERROR, "Failed to set socket options: " + std::string(strerror(errno)));
            close(sockfd);
            return;
        }

        struct sockaddr_in listen_addr;
        memset(&listen_addr, 0, sizeof(listen_addr));
        listen_addr.sin_family = AF_INET;
        listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        // Try to bind to ports 50052-50062
        bool bound = false;
        for (int port = 50052; port <= 50062; ++port) {
            listen_addr.sin_port = htons(port);
            if (bind(sockfd, (struct sockaddr *)&listen_addr,
                     sizeof(listen_addr)) == 0) {
                Log::logMessage(Log::PEER_INFO,
                                     "Successfully bound to port " +
                                         std::to_string(port) +
                                         " for peer discovery.");
                bound = true;
                break;
            }
        }

        if (!bound) {
            Log::logMessage(
                Log::PEER_ERROR,
                "Failed to bind socket to any port in range 50052-50062: " +
                    std::string(strerror(errno)));
            close(sockfd);
            return;
        }

        char buffer[1024];
        struct sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);

        while (true) {
            int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr *)&peer_addr, &addr_len);
            if (len < 0) {
                Log::logMessage(
                    Log::PEER_ERROR,
                    "Failed to receive: " + std::string(strerror(errno)));
                continue;
            }
            buffer[len] = '\0';

            std::string message(buffer);
            auto pos1 = message.find(',');
            auto pos2 = message.find(',', pos1 + 1);
            auto pos3 = message.find(',', pos2 + 1);

            if (pos1 != std::string::npos && pos2 != std::string::npos &&
                pos3 != std::string::npos) {
                std::string type = message.substr(0, pos1);
                std::string received_node_id =
                    message.substr(pos1 + 1, pos2 - pos1 - 1);
                int grpc_port =
                    std::stoi(message.substr(pos2 + 1, pos3 - pos2 - 1));
                std::string sender_ip = message.substr(pos3 + 1);

                char peer_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(peer_addr.sin_addr), peer_ip,
                          INET_ADDRSTRLEN);

                std::string own_message = "discovery," + node_id_ + ",50051," + own_ip_;
                if (type == "discovery" && message != own_message) {
                    AddOrUpdatePeer(received_node_id, peer_ip, grpc_port);
                }
            }
        }
    }

    void AddOrUpdatePeer(const std::string &peer_id, const std::string &peer_ip,
                         int peer_port) {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        std::string peer_address = peer_ip + ":" + std::to_string(peer_port);

        if (peers_.find(peer_id) == peers_.end()) {
            PeerInfo new_peer;
            new_peer.node_id = peer_id;
            new_peer.ip_address = peer_ip;
            new_peer.grpc_port = peer_port;
            new_peer.last_seen = std::chrono::system_clock::now();
            new_peer.channel = grpc::CreateChannel(
                peer_address, grpc::InsecureChannelCredentials());
            new_peer.stub = Messenger::NewStub(new_peer.channel);
            peers_[peer_id] = std::move(new_peer);
            Log::logMessage(
                Log::PEER_INFO,
                "Added new peer: " + peer_id + " at " + peer_address);
        } else {
            peers_[peer_id].last_seen = std::chrono::system_clock::now();
            Log::logMessage(
                Log::PEER_INFO,
                "Updated existing peer: " + peer_id + " at " + peer_address);
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
                        Log::logMessage(Log::PEER_WARN,
                                             "Peer " + it->first +
                                                 " is unresponsive. Removing.");
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
            for (const auto &peer : peers_) {
                MessengerClient client(peer.second.channel);
                // Check connectivity before updating topology
                if (!client.Heartbeat()) {
                    Log::logMessage(Log::PEER_WARN, "Peer " + peer.first + " is unresponsive. Skipping topology update.");
                    continue;
                }
                int retries = 3;
                while (retries > 0) {
                    if (client.UpdateTopology(topology)) {
                        break;
                    }
                    Log::logMessage(Log::PEER_WARN, "Retrying topology update for peer: " + peer.first);
                    retries--;
                    std::this_thread::sleep_for(std::chrono::seconds(2));  // Backoff before retrying
                }
                if (retries == 0) {
                    Log::logMessage(Log::PEER_ERROR, "Failed to update topology for peer: " + peer.first);
                }
            }
        }
    }

    std::string GetNetworkTopology() {
        std::stringstream ss;
        std::lock_guard<std::mutex> lock(peers_mutex_);
        ss << node_id_ << "," << own_ip_ << ",50051;";
        for (const auto &peer : peers_) {
            ss << peer.second.node_id << "," << peer.second.ip_address << ","
               << peer.second.grpc_port << ";";
        }
        return ss.str();
    }

    void getOwnIpAddress(char *ip) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        const char *kGoogleDnsIp = "8.8.8.8";
        uint16_t kDnsPort = 53;
        struct sockaddr_in serv;
        memset(&serv, 0, sizeof(serv));
        serv.sin_family = AF_INET;
        serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
        serv.sin_port = htons(kDnsPort);

        if (connect(sock, (const struct sockaddr *)&serv, sizeof(serv)) != 0) {
            Log::logMessage(
                Log::PEER_ERROR,
                "Connect failed: " + std::string(strerror(errno)));
            strcpy(ip, "127.0.0.1");
            return;
        }

        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if (getsockname(sock, (struct sockaddr *)&name, &namelen) != 0) {
            Log::logMessage(
                Log::PEER_ERROR,
                "Connect failed: " + std::string(strerror(errno)));
            strcpy(ip, "127.0.0.1");
        } else {
            inet_ntop(AF_INET, &name.sin_addr, ip, INET_ADDRSTRLEN);
        }

        close(sock);
    }

    std::string node_id_;
    char own_ip_[INET_ADDRSTRLEN];
    std::unique_ptr<ServerCompletionQueue> cq_;
    Messenger::AsyncService service_;

    // New ModelService AsyncService for FlatBuffers
    myservice::ModelService::AsyncService model_service_;

    std::unique_ptr<Server> server_;
    std::map<std::string, PeerInfo> peers_;
    std::mutex peers_mutex_;
};

int main(int argc, char **argv) {
    ServerImpl server;
    server.Run();
    return 0;
}
