#include <cstdint>	// uint64_t...
#include <vector>	// std::vector...

struct dang
{
	dang(const std::vector<uint64_t>& cho)
	{
	};
};

template<typename T>
struct gptr
{
	gptr();
};

int main()
{
	std::vector<uint64_t> beep;
	gptr<dang(beep)> gay;
	return 0;
}
