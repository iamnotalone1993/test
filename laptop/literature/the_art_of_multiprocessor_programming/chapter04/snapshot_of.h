#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

template<typename T>
class stamped_value
{
public:
	uint64_t	stamp;
	T		value;

	stamped_value(const uint64_t& ts = 0, const T& v)
		: stamp{ts}, value{v} {}
};

template<typename T>
bool equals(stamped_value<T> *x, stamped_value<T> *y, const int& length)
{
	for (uint64_t i = 0; i < length; ++i)
		if (x[i] != y[i])
			return false;
	return true;
}

template<typename T>
class snapshot_of
{
public:
	std::atomic<stamped_value<T>>	*a_table;
	uint64_t			length;

	snapshot_of(const uint64_t& capacity, const T& init) : length{capacity}
	{
		a_table = new std::atomic<stamped_value<T>>[length];
		for (uint64_t i = 0; i < length; ++i)
			a_table[i].store(init);
	}

	~snapshot_of()
	{
		delete[] a_table;
	}

	void update(const T& value)
	{
		stamped_value<T> value_old = a_table[TID].load();
		stamped_value<T> value_new = {value_old.stamp + 1, value};
		a_table[TID].store(value_new);
	}

	stamped_value<T>* collect()
	{
		copy = new stamped_value<T>[length];
		for (uint64_t i = 0; i < length; ++i)
			copy[i] = a_table[i].load();
		return copy;
	}

	void scan(T* result, uint64_t& size)
	{
		size = length;
		stamped_value<T> *copy_old, *copy_new;
		copy_old = collect();
	collect:
		while (true)
		{
			copy_new = collect();
			if (!equals(copy_old, copy_new, length))
			{
				for (uint64_t i = 0; i < length; ++i)
					copy_old[i] = copy_new[i];
				delete[] copy_new;
				continue collect;
			}
			T* result = new T[length];
			for (uint64_t i = 0; i < length; ++i)
				result[i] = copy_new[i].value;
			delete[] copy_old;
			return;
		}
	}
};
