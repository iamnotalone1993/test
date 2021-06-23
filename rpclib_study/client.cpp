#include <iostream>
#include "rpc/client.h"

struct task
{
        int                     a;
        std::vector<int>        v;

        MSGPACK_DEFINE(a,v);
};

int main()
{
	rpc::client c("localhost", 8080);

	task t;
	t.a = 1;
	t.v.push_back(100);
	t.v.push_back(200);
	t.v.push_back(300);

	for (int i = 0; i < 10; ++i)
		c.call("echo", t);

	return 0;
}
