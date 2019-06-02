/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

bool pe_relocs::is_bad()
{
	return base->idd[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0;
}

reloc_dir *pe_relocs::directory(size_t id)
{
	uint32_t counter = 0;
	uint32_t offset = base->rva_to_offset(base->idd[5].VirtualAddress);
	reloc_dir *reloc = reinterpret_cast<reloc_dir*>(base->data + offset);
	while (reloc->SizeOfBlock && reloc->VirtualAddress) {
		if (counter++ == id) return reloc;
		reloc = reinterpret_cast<reloc_dir*>(base->data + (offset += reloc->SizeOfBlock));
	}
	return nullptr;
}

reloc_data *pe_relocs::get_block(size_t id)
{
	reloc_dir *dir = directory(id);
	if (dir == nullptr) return nullptr;
	return &reloc_data(base, dir);
}

size_t pe_relocs::blocks_count()
{
	size_t counter = 0;
	while (directory(counter)) counter++;
	return counter;
}

bool pe_relocs::add_reloc(size_t block_addr, uint16_t offs, uint8_t type)
{
	if (is_bad()) return false;

	uint32_t start = 0;
	uint32_t offset = start = base->rva_to_offset(base->idd[5].VirtualAddress);
	reloc_dir *reloc = reinterpret_cast<reloc_dir*>(base->data + offset);
	while (reloc->SizeOfBlock && reloc->VirtualAddress) 
	{
		if (reloc->VirtualAddress != block_addr) {
			reloc = reinterpret_cast<reloc_dir*>(base->data + (offset += reloc->SizeOfBlock));
			continue;
		}

		offset += reloc->SizeOfBlock;
		uint16_t *last = reinterpret_cast<uint16_t*>(base->data + offset - 2);
		if (*last == 0) {
			*last |= offs; *last |= type << 12;
		} 
		else {
			uint32_t remaining_size = base->idd[5].Size - (offset - start);
			uint8_t *data = new uint8_t[remaining_size];
			memcpy(data, base->data + offset, remaining_size);
			memset(base->data + offset, 0, remaining_size);
			memcpy(base->data + offset + 4, data, remaining_size);
			delete[] data;
			
			last++; *last |= offs; *last |= type << 12;
			reloc->SizeOfBlock += 4; base->idd[5].Size += 4;
		}
		break;
	}
	return true;
}

// --------------- class reloc_data --------------- //
size_t reloc_data::get_address()
{
	return reloc->VirtualAddress;
}

uint8_t reloc_data::get_type(size_t id)
{
	if (id < 0 || id > get_count()) return 0;
	if (reloc == nullptr) return 0;
	return reinterpret_cast<uint16_t *>(reloc + 1)[id] >> 12;
}

uint16_t reloc_data::get_offset(size_t id)
{
	if (id < 0 || id > get_count()) return 0;
	if (reloc == nullptr) return 0;
	return reinterpret_cast<uint16_t *>(reloc + 1)[id] & 0xFFF;
}

size_t reloc_data::get_count()
{
	return (reloc->SizeOfBlock - sizeof(reloc_dir)) / sizeof(uint16_t);
}