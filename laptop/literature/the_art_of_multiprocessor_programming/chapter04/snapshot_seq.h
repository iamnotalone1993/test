#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

template<typename T>
class snapshot_seq
{
public:
	std::atomic<T>	*a_value;
	uint64_t	length;

	snapshot_seq(const uint64_t& capacity, const T& init) : length{capacity}
	{
		a_value = new std::atomic<T>[length];
		for (uint64_t i = 0; i < length; ++i)
			a_value[i] = init;
	}

	void update(const T& v)
	{
		a_value[TID] = v;
	}

	void scan(std::atomic<T>*& result, uint64_t& size)
	{
		size = length;
		T* result = new T[length];
		for (uint64_t i = 0; i < length; ++i)
			result[i] = a_value[i];
	}
};
