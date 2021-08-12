#include <hwloc.h>
#include <iostream>

static void print_children(hwloc_topology_t topology, hwloc_obj_t obj, int depth);

int main()
{
	hwloc_topology_t	topology;
	int			topodepth,
				depth,
				i;
	char			string[128];
	hwloc_obj_t		obj;

	hwloc_topology_init(&topology);

	hwloc_topology_load(topology);

	int 		levels = 0;
	unsigned long	size;

	for (obj = hwloc_get_obj_by_type(topology, HWLOC_OBJ_PU, 0); obj; obj = obj->parent)
		if (obj->type == HWLOC_OBJ_CACHE)
		{
			++levels;
			size += obj->attr->cache.size;
		}
	std::cout << "*** Logical processor 0 has " << levels << " caches totaling " << size / 1024 << "KB\n";

	hwloc_topology_destroy(topology);

	return 0;
}

static void print_children(hwloc_topology_t topology, hwloc_obj_t obj, int depth)
{
	char		type[32],
			attr[1024];
	unsigned	i;

	hwloc_obj_type_snprintf(type, sizeof(type), obj, 0);
	printf("%*s%s", 2 * depth, "", type);

	if (obj->os_index != (unsigned) - 1)
		printf("#%u", obj->os_index);

	hwloc_obj_attr_snprintf(attr, sizeof(attr), obj, " ", 0);
	if (*attr)
		printf("(%s)", attr);
	printf("\n");
	for (i = 0; i < obj->arity; ++i)
		print_children(topology, obj->children[i], depth + 1);
}
