/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

bool pe_import::is_bad()
{
	return base->idd[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0;
}

import_dir *pe_import::descriptor(size_t id)
{
	uint32_t offset = base->rva_to_offset(base->idd[1].VirtualAddress);
	return &reinterpret_cast<import_dir *>(base->data + offset)[id];
}

char *pe_import::get_library_name(size_t id)
{
	uint32_t offset = base->rva_to_offset(descriptor(id)->Name);
	return reinterpret_cast<char*>(base->data + offset);
}

size_t pe_import::get_library_count()
{
	static size_t iterator = 0;
	if (!iterator) { while (descriptor(iterator)->Name) iterator++; }
	return iterator;
}

size_t pe_import::get_func_count(size_t id)
{
	return get_data(id)->get_count();
}

import_thunk *pe_import::get_first_thunk(size_t id)
{
	uint32_t thunk_offset = 0;
	if (descriptor(id)->Characteristics)
		thunk_offset = base->rva_to_offset(descriptor(id)->OriginalFirstThunk);
	else thunk_offset = base->rva_to_offset(descriptor(id)->FirstThunk);
	return reinterpret_cast<import_thunk*>(base->data + thunk_offset);
}

import_data *pe_import::get_data(size_t id)
{
	return &import_data(base, reinterpret_cast<uint32_t*>(get_first_thunk(id)));
}

// --------------- class import_data --------------- //
bool import_data::is_by_name()
{
	return !IMAGE_SNAP_BY_ORDINAL(*thunk);
}

size_t import_data::get_count()
{
	uint32_t iterator = 0;
	while (thunk[iterator]) iterator++;
	return iterator;
}

size_t import_data::get_ordinal(size_t id)
{
	if (is_by_name()) return 0;
	return IMAGE_ORDINAL(thunk[id]);
}

char *import_data::get_name(size_t id)
{
	if (!is_by_name()) {
		char buffer[128];
		return itoa(get_ordinal(id), buffer, 10);
	}
	uint32_t offset = base->rva_to_offset(thunk[id]) + 2;
	return reinterpret_cast<char*>(base->data + offset);
}

uint16_t import_data::get_index(size_t id)
{
	if (!is_by_name()) return 0;

	uint32_t offset = base->rva_to_offset(thunk[id]);
	return *reinterpret_cast<uint16_t *>(base->data + offset);
}