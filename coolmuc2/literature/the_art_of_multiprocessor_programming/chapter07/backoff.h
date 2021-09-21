#include <random>	// std::default_random_engine...
#include <algorithm>	// std::min...
#include <thread>	// std::this_thread::sleep_for...

class backoff
{
public:
	backoff(const int& min, const int& max) : 
		delay_min{min}, delay_max{max}, limit{delay_min} {}

	void delay()
	{
		std::default_random_engine generator;
		std::uniform_int_distribution<int> distribution(0, limit - 1);	// [0, limit)
		int amount = distribution(generator);
		limit = std::min(delay_max, 2 * limit);
		std::this_thread::sleep_for(std::chrono::milliseconds(amount));
	}

private:
	int	delay_min;
	int	delay_max;
	int	limit;
};
