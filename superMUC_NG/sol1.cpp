#include <omp.h>
#include <iostream>
#include <chrono>

inline double trial(const int &N, const int &REP);

int main()
{
	std::cout << trial(1000, 1000) << " MFlops/sec\n";

	return 0;
}

inline double trial(const int &N, const int &REP)
{
	double *a = new double[N];
	double *b = new double[N];
	double *c = new double[N];
	double *d = new double[N];

	for (int i = 0; i < N; ++i)
	{
		a[i] = 0;
		b[i] = i;
		c[i] = i + 1;
		d[i] = i + 2;
	}
	
	auto t0 = std::chrono::high_resolution_clock::now();

	for (int j = 0; j < REP; ++j)
		for (int i = 0; i < N; ++i)
			a[i] = b[i] + c[i] * d[i];

	auto t1 = std::chrono::high_resolution_clock::now();

	delete[] a;
	delete[] b;
	delete[] c;
	delete[] d;
	
	using dsec = std::chrono::duration<double>;
	double dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

	double flop = 2.0 * (double) N * (double) REP * 1.0e-6;

	return flop/dur;
}
