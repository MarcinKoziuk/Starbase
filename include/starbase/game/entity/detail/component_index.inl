int g_component_type_index_counter;

template<typename T>
static int component_index()
{
	static int index = g_component_type_index_counter++;
	return index;
}
