#include <atomic>	// std::atomic...
#include "backoff.h"	// backoff...

class lock_ttas_bo
{
public:
	lock_ttas_bo() : state{false} {}

	lock()
	{
		backoff bo(DELAY_MIN, DELAY_MAX);
		while (true)
		{
			while (state.load());
			if (!state.exchange(true))
				return;
			else
				backoff.delay();
		}
	}

	unlock()
	{
		state.store(false);
	}

private:
	std::atomic<bool> 	state;
	const int		DELAY_MIN = ...;
	const int		DELAY_MAX = ...;
};
