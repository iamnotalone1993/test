#include <atomic>	// std::atomic_flag...

class lock_tas
{
public:
	lock_tas() : state{ATOMIC_FLAG_INIT} {}

	lock()
	{
		while (state.test_and_set());
	}

	unlock()
	{
		state.clear();
	}

private:
	std::atomic_flag state;
};
