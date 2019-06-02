/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>


bool pe_section::is_bad(size_t id)
{
	if (id >= base->inh->FileHeader.NumberOfSections)
		return true;
	return base->ish[id].SizeOfRawData == 0;
}

size_t pe_section::get_count()
{
	return base->inh->FileHeader.NumberOfSections;
}

sec_header *pe_section::get(size_t id)
{
	if (is_bad(id)) return nullptr;
	return &base->ish[id];
}

sec_header *pe_section::get(char *name)
{
	for (size_t i = 0; i < base->inh->FileHeader.NumberOfSections; i++) {
		if (!strcmp((char*)base->ish[i].Name, name)) {
			return &base->ish[i];
		}
	}
	return nullptr;
}

sec_header *pe_section::add(char *name, size_t size, size_t foffs, size_t flags)
{
	uint16_t count = base->inh->FileHeader.NumberOfSections;

	strncpy((char *)base->ish[count].Name, name, 8);
	base->ish[count].PointerToRawData = foffs;
	base->ish[count].Misc.VirtualSize = size;
	base->ish[count].VirtualAddress = base->size_of_image();
	base->ish[count].SizeOfRawData = base->align(size, base->inh->OptionalHeader.FileAlignment);
	base->ish[count].Characteristics = flags;

	base->inh->OptionalHeader.SizeOfImage = base->size_of_image();
	base->inh->FileHeader.NumberOfSections++;

	return &base->ish[count];
}

char *pe_section::get_name(size_t id)
{
	if (is_bad(id)) return nullptr;
	return reinterpret_cast<char*>(base->ish[id].Name);
}

char *pe_section::get_data(size_t id)
{
	if (is_bad(id)) return nullptr;
	return reinterpret_cast<char*>(base->data + base->ish[id].PointerToRawData);
}

char *pe_section::get_data(char *name)
{
	return reinterpret_cast<char*>(base->data + get(name)->PointerToRawData);
}
