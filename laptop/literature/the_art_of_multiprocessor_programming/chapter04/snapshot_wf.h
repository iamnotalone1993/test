#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

template<typename T>
class stamped_snap
{
public:
	uint64_t	stamp;
	T		value;
	T		*snap;

	stamped_value(const uint64_t& ts = 0, const T& v, T* s = nullptr)
		: stamp{ts}, value{v}, snap{s} {}
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
class snapshot_wf
{
public:
	std::atomic<stamped_snap<T>>	*a_table;
	uint64_t			length;

	snapshot_wf(const uint64_t& capacity, const T& init) : length{capacity}
	{
		a_table = new std::atomic<stamped_snap<T>>[length];
		for (uint64_t i = 0; i < length; ++i)
			a_table[i].store(init);
	}

	~snapshot_wf()
	{
		delete[] a_table;
	}

	void update(const T& value)
	{
		T *snap = new T[length];
		uint64_t size;
		scan(snap, size);
		stamped_snap<T> value_old = a_table[TID].load();
		stamped_snap<T> value_new = {value_old.stamp + 1, value, snap};
		a_table[TID].store(value_new);
		delete[] snap;
	}

	stamped_value<T>* collect()
	{
		copy = new stamped_snap<T>[length];
		for (uint64_t i = 0; i < length; ++i)
			copy[i] = a_table[i].load();
		return copy;
	}

	void scan(T* result, uint64_t& size)
	{
		size = length;
		stamped_value<T> *copy_old, *copy_new;
		bool moved = new bool[length];
		for (uint64_t i = 0; i < length; ++i)
			moved = false;

		copy_old = collect();
	collect:
		while (true)
		{
			copy_new = collect();
			for (uint64_t i = 0; i < length; ++i)
				if (copy_old[i].stamp != copy_new[i].stamp)
					if (moved[i])
					{
						result = copy_new[i].snap;
						delete[] moved;
						delete[] copy_old;
						delete[] copy_new;
						return;
					}
					else
					{
						moved[i] = true;
						for (uint64_t i = 0; i < length; ++i)
							copy_old[i] = copy_new[i];
						delete[] copy_new;
						continue collect;
					}
			T* result = new T[length];
			for (uint64_t i = 0; i < length; ++i)
				result[i] = copy_new[i].value;
			delete[] moved;
			delete[] copy_old;
			return;
		}
	}
};
