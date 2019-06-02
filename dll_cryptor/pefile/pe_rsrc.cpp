/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

bool pe_rsrc::is_bad()
{
	return base->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size == 0;
}

rsrc_dir *pe_rsrc::directory()
{
	size_t offset = base->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	return reinterpret_cast<rsrc_dir*>(base->data + base->rva_to_offset(offset));
}

rsrc_entry *pe_rsrc::get_entry(size_t id)
{
	if (id >= get_count()) return nullptr;
	size_t offset = base->rva_to_offset(base->idd[2].VirtualAddress) + sizeof(rsrc_dir);
	return &rsrc_entry(base, reinterpret_cast<dir_entry*>(base->data + offset + (id * sizeof(dir_entry))));
}

size_t pe_rsrc::get_count()
{
	return directory()->NumberOfIdEntries + directory()->NumberOfNamedEntries;
}

rsrc_data *pe_rsrc::find_rsrc(size_t id)
{
	for (size_t i = 0; i < get_count(); i++) {
		rsrc_entry *type = get_entry(i);
		for (size_t x = 0; x < type->get_count(); x++) {
			rsrc_entry *rsrc = type->get_entry(x);
			if (rsrc->is_named()) continue;
			if (id == rsrc->get_id()) {
				return rsrc->get_entry(0)->get_data();
			}
		}
	}
	return nullptr;
}

rsrc_data *pe_rsrc::find_rsrc(char *name)
{
	for (size_t i = 0; i < get_count(); i++) {
		rsrc_entry *type = get_entry(i);
		for (size_t x = 0; x < type->get_count(); x++) {
			rsrc_entry *rsrc = type->get_entry(x);
			if (!rsrc->is_named()) continue;
			if (!strcmp(rsrc->get_name(), name)) {
				return rsrc->get_entry(0)->get_data();
			}
		}
	}
	return nullptr;
}

// --------------- class rsrc_entry --------------- //
bool rsrc_entry::is_named()
{
	return entry->Name & IMAGE_RESOURCE_NAME_IS_STRING;
}

bool rsrc_entry::is_directory()
{
	return entry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY;
}

char *rsrc_entry::get_name()
{
	static char buffer[260];
	if (is_named() == false) return nullptr;
	size_t offset = base->rva_to_offset(base->idd[2].VirtualAddress);
	rsrc_str *rsrc = reinterpret_cast<rsrc_str*>(base->data + offset + entry->NameOffset);
	WideCharToMultiByte(1251, 0, rsrc->NameString, -1, buffer, rsrc->Length, 0, 0);
	return buffer;
}

size_t rsrc_entry::get_id()
{
	if (is_named() == true) return 0;
	return entry->Id;
}

rsrc_dir *rsrc_entry::get_dir()
{
	if (!is_directory()) return nullptr;
	size_t offset = base->rva_to_offset(base->idd[2].VirtualAddress);
	return reinterpret_cast<rsrc_dir*>(base->data + offset + entry->OffsetToDirectory);
}

rsrc_entry *rsrc_entry::get_entry(size_t id)
{
	if (!is_directory()) return nullptr;
	size_t offset = base->rva_to_offset(base->idd[2].VirtualAddress);
	offset += entry->OffsetToDirectory + sizeof(rsrc_dir) + id * sizeof(dir_entry);
	return &rsrc_entry(base, reinterpret_cast<dir_entry*>(base->data + offset));
}

size_t rsrc_entry::get_count()
{
	rsrc_dir *dir = get_dir();
	if (dir == nullptr) return 0;
	return dir->NumberOfIdEntries + dir->NumberOfNamedEntries;
}

rsrc_data *rsrc_entry::get_data()
{
	if (is_directory() == false) {
		size_t offset = base->rva_to_offset(base->idd[2].VirtualAddress);
		return reinterpret_cast<rsrc_data*>(base->data + offset + entry->OffsetToData);
	}
	return nullptr;
}