#include <iostream>
#include <vector>

int main()
{
	int *dang = new int;
	*dang = 5;

	std::vector<int *> vc;
	vc.push_back(dang);

	for (int i = 0; i < vc.size(); ++i)
		std::cout << "[1]" << *vc[i] << std::endl;

	vc.clear();

	delete dang;
	delete dang;

	std::cout << "[2]" << *dang << std::endl;

	return 0;
}
