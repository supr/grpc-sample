#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "helloworld.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class GreeterClient {
	public:
		GreeterClient(std::shared_ptr<Channel> channel): stub_(Greeter::NewStub(channel)) {}

		void SayHello(const std::string& user) {
			HelloRequest request;
			request.set_name(user);

			HelloReply reply;

			ClientContext context;

			/*Status status = stub_->SayHello(&context, request, &reply);
			if(status.ok()) {
				return reply.message();
			} else {
				std::cout << status.error_code() << ": " << status.error_message() << std::endl;
				return "RPC failed";
			}*/

			std::unique_ptr<ClientReader<HelloReply> > reader(
					stub_->SayHello(&context, request));
			while(reader->Read(&reply)) {
				std::cout << "Response: " << reply.message() << '\n';
			}
			Status status = reader->Finish();
			if(status.ok()) {
				std::cout << "SayHello rpc succeded." << std::endl;
			} else {
				std::cout << "SayHello rpc failed." << status.error_code() << " : " << status.error_message() << std::endl;
			}
		}

	private:
		std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
	GreeterClient gc(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
	std::string user;
	if(argc > 1) {
		user = std::string(argv[1]);
	} else {
		user = std::string("world");
	}

	gc.SayHello(user);

	return 0;
}
