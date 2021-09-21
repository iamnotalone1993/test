#include <atomic>	// std::atomic_flag...
#include "backoff.h"	// backoff...

class lock_tas_bo
{
public:
	lock_tas_bo() : state{ATOMIC_FLAG_INIT} {}

	lock()
	{
		backoff bo(DELAY_MIN, DELAY_MAX);
		while (true)
		{
			if (!state.test_and_set())
				return;
			else
				backoff.delay();
		}
	}

	unlock()
	{
		state.clear();
	}

private:
	std::atomic_flag 	state;
	const int		DELAY_MIN = ...;
	const int		DELAY_MAX = ...;
};
