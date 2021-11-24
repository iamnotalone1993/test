#ifndef LOCAL_HISTRORY_H
#define LOCAL_HISTRORY_H

#include <cmath>	// ceil...

struct subepoch
{
	uint64_t	one;	// one
	uint64_t	reset;	// assign one to corresponding subepoch of calling thread;
	uint64_t	seq[3];	// a sequence of one, two, or three

	uint64_t	value;	// its value
};

class local_history
{
public:
	uint32_t				count;		// its count

	local_history(const uint64_t& num_threads, const uint64_t& tid, const tree_abs& tree);
	~local_history();
	void update(std::vector<void *> *retired);

private:
	std::vector<std::atomic<uint64_t> *>	gepoch;		// its view of global epoch
	subepoch				*lepoch;	// its local epoch
	const uint32_t				LEVEL;		// its number of levels
	uint32_t				curr;		// its location
	bool					last;		// is it the last one?

	bool is_last();
};

local_history::local_history(	const uint64_t& 	num_threads,
				const uint64_t& 	tid,
				const tree_abs& 	tree)
		: LEVEL{tree.height},
		count{0},
		curr{0},
		last{true},
		lepoch{new subepoch[tree.height]}
{
	uint32_t degree[LEVEL], tid_level, tid_next_level, num_threads_level, pos, tmp, degree_lepoch;

	for (int i = 0; i < LEVEL; ++i)
	{
		if (i == 0)
		{
			degree[i] = ceil(float(num_threads) / tree.width[0]);
			tid_level = tid;
			tid_next_level = tid / degree[i];
			num_threads_level = num_threads;
		}
		else // if (i > 0)
		{
			degree[i] = ceil(float(tree.width[i-1]) / tree.width[i]);
			tid_level = tid_next_level;
			tid_next_level /= degree[i];
			num_threads_level = ceil(float(num_threads_level) / degree[i-1]);
		}

		gepoch.push_back(&tree.node[i][tid_next_level]);

		pos = tid_level % degree[i] * 2;
		lepoch[i].one = 1 << pos;
		lepoch[i].reset = ~(1 << (pos + 1));
		lepoch[i].value = 0;

		tmp = (tree.width[i] - 1) * degree[i];
		if (tid_level < tmp)
			degree_lepoch = degree[i];
		else // if (tid_level >= tmp)
			degree_lepoch = num_threads_level - tmp;

		for (int j = 0; j < 3; ++j)
			lepoch[i].seq[j] = 0;
		for (int j = 0; j < degree_lepoch; ++j)
			for (int k = 0; k < 3; ++k)
				lepoch[i].seq[k] += k + 1 << 2 * j;
	}
}

local_history::~local_history()
{
	delete[] lepoch;
}

void local_history::update(std::vector<void *> *retired)
{
	if (last)
	{
		if (count < 3)
		{
			// one RMA
			lepoch[curr].value = gepoch[curr]->fetch_add(lepoch[curr].one);

			lepoch[curr].value += lepoch[curr].one;
		}
		else // if (count == 3)
		{
			// one RMA
			lepoch[curr].value = gepoch[curr]->fetch_and(lepoch[curr].reset);

			lepoch[curr].value &= lepoch[curr].reset;
			if (curr == 0)
				count = 0;
		}
		if (curr == 0)
		{
			while (!retired[count].empty())
			{
				delete (int*)retired[count].back();
				retired[count].pop_back();
			}
			++count;
		}
		last = is_last();
	}
	else // if (!last)
	{
		// one RMA
		lepoch[curr].value = gepoch[curr]->load();

		if (lepoch[curr].value == lepoch[curr].seq[count-1])
		{
			if (curr == LEVEL - 1)
				last = true;
			curr = (curr + 1) % LEVEL;
		}
		else if (lepoch[curr].value > lepoch[curr].seq[count-1])
			last = true;
	}
}

bool local_history::is_last()
{
	if (lepoch[curr].value == lepoch[curr].seq[count-1])
	{
		curr = (curr + 1) % LEVEL;
		return true;
	}
	return false;
}

#endif /* LOCAL_HISTORY_H */
