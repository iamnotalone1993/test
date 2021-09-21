#include <atomic>	// std::atomic_flag...

class lock_ttas
{
public:
	lock_ttas() : state{false} {}

	lock()
	{
		while (true)
		{
			while (state.load());		// wait until state == false
			if (!state.exchange(true))	// exchange here means test_and_set
				return;
		}
	}

	unlock()
	{
		state.store(false);
	}

private:
	std::atomic<bool> state;
;
