#include <iostream>
#include <vector>
#include <cstddef>
#include "rpc/server.h"

struct task
{
	int 			a;
	std::vector<int> 	v;

	MSGPACK_DEFINE(a,v);
};

int main()
{
	std::cout << "The server is running\n";

	rpc::server srv(8080);

	srv.bind("echo", [](task t)
			{
		       		std::cout << t.a << std::endl;
				while (!t.v.empty())
				{
					std::cout << t.v.back() << std::endl;
					t.v.pop_back();
				}
				std::cout << std::endl;	
			});

	constexpr size_t thread_count = 8;

	srv.async_run(thread_count);

	//for (int i = 0; i < 1000; ++i)
		std::cout << "Hello World\n";

	std::cin.ignore();

	return 0;
}
