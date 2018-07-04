#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;

using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class GreeterServiceImpl final: public Greeter::Service {
	//Status SayHello(ServerContext* context, const HelloRequest* request, HelloReply* reply) override {
	Status SayHello(ServerContext* context, const HelloRequest* request, ServerWriter<HelloReply>* writer) override {
		std::string prefix("Hello ");
		std::string user(request->name());
		std::cout << "Got new Request for: " << user << std::endl;
		for(int i = 0; i < 10; i++) {
			HelloReply h;
			h.set_message(prefix + user);
			writer->Write(h);
		}
		//reply->set_message(prefix + user);
		return Status::OK;
	}
};

void RunServer() {
	std::string server_address("0.0.0.0:50051");
	GreeterServiceImpl service;

	ServerBuilder builder;
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << server_address << std::endl;

	server->Wait();
}

int main(int argc, char** argv) {
	RunServer();

	return 0;
}
