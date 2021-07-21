#include <omp.h>
#include <iostream>

/* Interface */
template <typename T>
inline void store(const T& src, T& dst);

template <typename T>
inline T load(const T& src);

template <typename T>
inline void exchange(T& dst, T& val);

template <typename T>
inline T fetch_add(T& dst, const T& val);

template <typename T>
inline T fetch_sub(T& dst, const T& val);

template <typename T>
inline T fetch_and(T& dst, const T& val);

template <typename T>
inline T fetch_or(T& dst, const T& val);

template <typename T>
inline T fetch_xor(T &dst, const T& val);
/**/

/* Implementation */
template <typename T>
inline void store(const T& src, T& dst)
{
	#pragma omp atomic seq_cst write
		dst = src;
}

template <typename T>
inline T load(const T& src)
{
	T res;

	#pragma omp atomic seq_cst read
		res = src;

	return res;
}

template <typename T>
inline void exchange(T& dst, T& val)
{
	#pragma omp atomic seq_cst capture
	{
		dst = val;
		val = dst;
	}
}

template <typename T>
inline T fetch_add(T& dst, const T& val)
{
	T res;

	#pragma omp atomic seq_cst capture
	{
		res = dst;
		dst += val;
	}

	return res;
}

template <typename T>
inline T fetch_sub(T& dst, const T& val)
{
	T res;

	#pragma omp atomic seq_cst capture
	{
		res = dst;
		dst -= val;
	}

	return res;
}

template <typename T>
inline T fetch_and(T& dst, const T& val)
{
	T res;

	#pragma omp atomic seq_cst capture
	{
		res = dst;
		dst &= val;
	}

	return res;
}

template <typename T>
inline T fetch_or(T& dst, const T& val)
{
	T res;

	#pragma omp atomic seq_cst capture
	{
		res = dst;
		dst |= val;
	}

	return res;
}

template <typename T>
inline T fetch_xor(T& dst, const T& val)
{
	T res;

	#pragma omp atomic seq_cst capture
	{
		res = dst;
		dst ^= val;
	}

	return res;
}
/**/

int main(int argc, char **argv)
{
	int	nthreads,
		tid,
		shared_data = 0;

	#pragma omp parallel private(nthreads, tid) shared(shared_data)
	{
		nthreads = omp_get_num_threads();
		tid = omp_get_thread_num();

		fetch_add(shared_data, 1);
		#pragma omp barrier

		#pragma omp critical
			std::cout << "shared_data = " << shared_data << std::endl;
	}

	return 0;
}
