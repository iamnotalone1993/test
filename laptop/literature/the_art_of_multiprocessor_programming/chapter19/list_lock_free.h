#include <atomic>		// std::atomic...
#include <cstdint>		// uintptr_t...
#include <limits>		// std::numeric_limits...
#include "memory_manager.h"	// mem_manager...

template<typename T>
class node
{
public:
	node(const T& item): *this{{item, nullptr}} {};
	node(const T& item, const node<T>*&next): *this{{item, next}} {};

	// return whether the low bit of the pointer is marked or not
	bool is_marked() { return (uintptr_t(this) & 1); }

	// clear the mark bit of a pointer
	node* unmark() { (node*(uintptr_t(this) & ~uintptr_t(1))); }

	// set the mark bit of a pointer
	node* mark() { (node*(uintptr_t(this) | 1)); }

private:
	T			item;
	std::atomic<node<T>*>	next;
};

/* Interface */
template<typename T>
class list_lf<T>
{
public:
	list_lf();
	bool add(const T& item);
	bool remove(const T& item);
	bool contains(const T& item);

private:
	node*		head;
	mem_manager*	mm;
	
	bool find(const T& item, node<T>*& prev, node<T>*& curr, node<T>*& next);
};

/* Implementation */
template<typename T>
list_lf<T>::list_lf()
{
	head = new node(std::numeric_limits<int>::min());
	head->next = new node(std::numeric_limits<int>::max());
}

template<typename T>
bool list_lf<T>::add(const T& item)
{
	mm->op_begin();
	node<T> *prev, *curr, *next;
	while (true)
	{
		if (find(key, prev, curr, next, mm)
		{
			mm->op_end();
			return false;
		}
		node<T>* new_node = new node<T>(item, curr);
		if (prev->next.compare_exchange_strong(curr, new_node))
		{
			mm->op_end();
			return true;
		}
		mm->unreserve(prev);
		mm->unreserve(curr);
	}
}

template<typename T>
bool list_lf<T>::remove(const T& item)
{
	mm->op_begin();
	node<T> *prev, *curr, *next;
	while (true)
	{
		if (!find(key, prev, curr, next, mm))
		{
			mm->op_end();
			return false;
		}
		if (!curr->next.compare_exchange_strong(next, mark(next)))
		{
			mm->unreserve(prev);
			mm->unreserve(curr);
			continue;
		}
		if (prev->next.compare_exchange_strong(curr, next))
			mm->sched_for_reclaim(curr);
		else
		{
			mm->unreserve(prev);
			mm->unreserve(curr);
			find(key, prev, curr, next);
		}
		mm->op_end();
		return true;
	}
}

template<typename T>
bool list_lf<T>::contains(const T& item)
{
	mm->op_begin();
	node<T> *prev, *curr, *next;
	bool ans = find(key, prev, curr, next);
	mm->op_end();
	return ans;
}

template<typename T>
bool list_lf<T>::find(const T& item, node<T>*& prev, node<T>*& curr, node<T>*& next)
{
	prev = list;
	mm->try_reserve(prev);
	curr = prev->next.load();
	while (curr != nullptr)
	{
		if (mm->try_reserve(curr))
		{
			if (prev->next.load() != curr)
			{
				mm->unreserve(prev);
				mm->unreserve(curr);
				return find(item, prev, curr, next, mm);
			}
		}
		next = curr->next.load();
		if (next->is_marked())	// curr is logically deleted
		{
			node<T>* tmp = next->unmark();
			if (!prev->next.compare_exchange_strong(curr, tmp))
			{
				mm->unreserve(prev);
				mm->unreserve(curr);
				return find(item, prev, curr, next, mm);
			}
			mm->unreserve(curr);
			mm->sched_for_reclaim(curr);
			curr = tmp;
		}
		else
		{
			T citem = curr->item;
			if (prev->next.load() != curr)
			{
				mm->unreserve(prev);
				mm->unreserve(curr);
				return find(key, prev, curr, next);
			}
			if (citem >= item)
				return (citem == item);
			mm->unreserve(prev);
			prev = curr;
			curr = next;
		}
	}
	return false;
}

