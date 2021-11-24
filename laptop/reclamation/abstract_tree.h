#ifndef ABSTRACT_TREE_H
#define ABSTRACT_TREE_H

class tree_abs
{
public:
	uint32_t		height;	// the height of the tree
	uint32_t		*width;	// the width of the tree at levels
	std::atomic<uint64_t>	**node;	// the value of tree nodes

	tree_abs();
	~tree_abs();
};

tree_abs::tree_abs()
{
	// TODO: Make this initialization more auto
	height = 3;
	width = new uint32_t[height];
	width[0] = 4;
	width[1] = 2;
	width[2] = 1;
	node = new std::atomic<uint64_t>*[height];
	for (int i = 0; i < height; ++i)
	{
		node[i] = new std::atomic<uint64_t>[width[i]];
		for (int j = 0; j < width[i]; ++j)
			node[i][j] = 0;
	}

	/*/ Debugging
	for (int i = 0; i < height; ++i)
		for (int j = 0; j < width[i]; ++j)
			std::cout << node[i][j].load() << std::endl;*/
}

tree_abs::~tree_abs()
{
	for (int i = 0; i < height; ++i)
		delete node[i];
	delete[] node;
	delete[] width;
}

#endif /* ABSTRACT_TREE_H */

