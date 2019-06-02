/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

bool pe_export::is_bad()
{
	return base->idd[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0;
}

export_dir *pe_export::directory()
{
	size_t offset = base->idd[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	return reinterpret_cast<export_dir*>(base->data + base->rva_to_offset(offset));
}

char *pe_export::get_func_name(size_t id)
{
	if (id < 0 || id >= get_name_count()) return NULL;
	uint32_t *names_ptr = get_names_ptr();
	return reinterpret_cast<char*>(base->data + base->rva_to_offset(names_ptr[id]));
}

size_t pe_export::get_func_address(size_t id)
{
	if (id < 0 || id >= get_func_count()) return NULL;
	uint32_t *funcs_ptr = get_funcs_ptr();
	return funcs_ptr[id];
}

char *pe_export::get_name_ptr() 
{
	return reinterpret_cast<char *>(base->data + base->rva_to_offset(directory()->Name));
}

uint32_t *pe_export::get_funcs_ptr()
{
	return reinterpret_cast<uint32_t *>(base->data + base->rva_to_offset(directory()->AddressOfFunctions));
}

uint16_t *pe_export::get_ordinals_ptr()
{
	return reinterpret_cast<uint16_t *>(base->data + base->rva_to_offset(directory()->AddressOfNameOrdinals));
}

uint32_t *pe_export::get_names_ptr()
{
	return reinterpret_cast<uint32_t *>(base->data + base->rva_to_offset(directory()->AddressOfNames));
}

size_t pe_export::get_func_count()
{
	return directory()->NumberOfFunctions;
}

size_t pe_export::get_name_count()
{
	return directory()->NumberOfNames;
}

void pe_export::create_dir(char *lib_name, size_t func_addr, char *func_name)
{
	if (base->idd[0].Size == 0) base->idd[0].Size = base->inh->OptionalHeader.FileAlignment;

	size_t va		= base->idd[0].VirtualAddress;
	size_t offset	= base->rva_to_offset(va);
	size_t begin	= base->rva_to_offset(va);
	export_dir temp = { 0, 0, 0, 0, va + 50, 1, 1, 1, va + 40, va + 44, va + 48 };
	size_t name_addr = va + 50 + strlen(lib_name) + 1;

	memset(base->data + offset, 0, base->idd[0].Size);	
	memmove(base->data + offset, &temp, sizeof(export_dir));		offset += sizeof(export_dir); 
	memmove(base->data + offset, &func_addr, sizeof(uint32_t));		offset += sizeof(uint32_t);
	memmove(base->data + offset, &name_addr, sizeof(uint32_t));		offset += sizeof(uint32_t) + 2;
	memmove(base->data + offset, lib_name, strlen(lib_name) + 1);	offset += strlen(lib_name) + 1;
	memmove(base->data + offset, func_name, strlen(func_name) + 1);	offset += strlen(func_name) + 1;
}

void pe_export::add_function(char *name, size_t address)
{
	if (is_bad()) return;

	size_t	offset		= 0;
	size_t	number		= get_func_count();
	size_t	*names_ptr	= get_names_ptr();
	size_t	last_func	= names_ptr[number - 1] + strlen(get_func_name(number - 1)) + 1;
	size_t	last_ord	= get_ordinals_ptr()[number - 1] + 1;
	uint8_t	*buf		= new unsigned char[base->idd[0].Size + 0x10];
	
	memcpy(buf + offset, directory(), sizeof(export_dir));	offset += sizeof(export_dir);
	memcpy(buf + offset, get_funcs_ptr(), number * 4);		offset += number * 4;
	memcpy(buf + offset, &address, sizeof(uint32_t));		offset += sizeof(uint32_t);
	memcpy(buf + offset, get_names_ptr(), number * 4);		offset += number * 4;
	memcpy(buf + offset, &last_func, sizeof(uint32_t));		offset += sizeof(uint32_t);
	memcpy(buf + offset, get_ordinals_ptr(), number * 2);	offset += number * 2;
	memcpy(buf + offset, &last_ord, sizeof(uint16_t));		offset += sizeof(uint16_t);
	memcpy(buf + offset, get_name_ptr(), strlen(get_name_ptr()) + 1); offset += strlen(get_name_ptr()) + 1;
	
	for (size_t i = 0; i < get_name_count(); i++) {
		memcpy(buf + offset, get_func_name(i), strlen(get_func_name(i)) + 1); 
		offset += strlen(get_func_name(i)) + 1;
	}
	strcpy((char*)buf + offset, name);

	offset = base->rva_to_offset(base->idd[0].VirtualAddress);
	memcpy(base->data + offset, buf, base->idd[0].Size + 10);

	directory()->NumberOfFunctions++; directory()->NumberOfNames++;
	directory()->AddressOfNames += 4; directory()->Name += 10;
	directory()->AddressOfNameOrdinals += 8;

	uint32_t *name_addr = get_names_ptr();
	for (size_t i = 0; i < get_name_count(); i++) name_addr[i] += 10;

	delete[] buf;
}
