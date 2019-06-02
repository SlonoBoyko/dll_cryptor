/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */


#include "main.h"

#define CRYPTED

void encrypt_file(std::string filename)
{
	string out_file = filename;
	size_t pos = out_file.find_last_of('.');
	out_file.insert(pos, ".crypted");

	pe_file *pefile = new pe_file(filename);

	std::vector<pair<uint32_t, string>> exp;
	std::string lib_name = pefile->get_export()->get_name_ptr();
	for (size_t i = 0; i < pefile->get_export()->get_func_count(); i++) {
		size_t func = pefile->get_export()->get_func_address(i);
		string name = pefile->get_export()->get_func_name(i);
		exp.push_back(pair<uint32_t, string>(pefile->rva_to_offset(func), name));
	}

	size_t file_align = pefile->inh->OptionalHeader.FileAlignment;
	size_t sect_align = pefile->inh->OptionalHeader.SectionAlignment;
	size_t old_filesz = pefile->size;
	
	size_t res_addres = pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	size_t res_length = pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
	size_t exp_length = pefile->idd[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	size_t end_export = pefile->align(exp_length + sizeof(loader), file_align);
	
	char *rsrc = nullptr;
	if (pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size) {
		rsrc = new char[pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size];
		memcpy(rsrc, pefile->get_section()->get_data(".rsrc"), res_length);
	}

#ifdef CRYPTED
	uint32_t *r_val = generate_key();
	uint32_t key[4] = { r_val[11], r_val[22], r_val[33], r_val[44] };
	for (int i = 0; i < pefile->size / 8; i++)
		xtea_encode(32, (uint32_t*)(pefile->data + (i*8)), key);
#else
	pefile->change_data(0x400, "\xC3", 1);
#endif

	pefile->insert_data(0, nullptr, end_export);
	pefile->change_data(0, loader, sizeof(loader));
	pefile->append_data(reloc, sizeof(reloc));
	if (res_length != 0 && rsrc != nullptr) {
		size_t size = pefile->align(res_length, file_align);
		pefile->append_data(rsrc, size); 
		delete rsrc;
	}
	pefile->load();

	size_t image_base = pefile->inh->OptionalHeader.ImageBase;	
	
	sec_header *sec			= pefile->get_section()->get(".data");
	size_t add_size			= old_filesz + exp_length;
	sec->SizeOfRawData		+= pefile->align(add_size, file_align);
	sec->Misc.VirtualSize	+= pefile->align(add_size, sect_align);

	sec = pefile->get_section()->get(".reloc");
	sec->VirtualAddress		+= pefile->align(add_size, sect_align);
	sec->PointerToRawData	+= pefile->align(add_size, file_align);
	pefile->idd[5].VirtualAddress = sec->VirtualAddress;

	pefile->change_data(0x4B9, old_filesz);
	pefile->change_data(0x4BE, image_base + pefile->offset_to_rva(end_export));
#ifdef CRYPTED
	pefile->change_data(0x1C00, key, sizeof(uint32_t) * 4);
	pefile->change_data(0x04B9, &old_filesz, sizeof(uint32_t));
#endif

	if (res_length > 0)
	{
		size_t rsrc_ptr = pefile->size - pefile->align(res_length, file_align);
		sec = pefile->get_section()->add(".rsrc", res_length, rsrc_ptr, 0x40000040);
		pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = sec->VirtualAddress;
		pefile->idd[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = sec->Misc.VirtualSize;

		for (size_t i = 0; i < pefile->get_rsrc()->get_count(); i++) {
			rsrc_entry *type = pefile->get_rsrc()->get_entry(i);
			for (size_t x = 0; x < type->get_count(); x++) {
				rsrc_entry *rsrc = type->get_entry(x);
				rsrc_data *rc = rsrc->get_entry(0)->get_data();
				size_t diff = rc->OffsetToData - res_addres;
				rc->OffsetToData = pefile->idd[2].VirtualAddress + diff;
			}
		}
	}

	if (exp_length > 0)
	{
		pefile->idd[IMAGE_DIRECTORY_ENTRY_EXPORT].Size = exp_length;
		pefile->idd[0].VirtualAddress = pefile->offset_to_rva(sizeof(loader));

		std::vector<pair<uint32_t, string>>::iterator it;
		for (it = exp.begin(); it != exp.end(); it++) {
			if (it != exp.begin()) pefile->get_export()->add_function(it->second.data(), it->first);
			else pefile->get_export()->create_dir(lib_name.data(), it->first, exp.begin()->second.data());
		}
	}

	pefile->inh->OptionalHeader.SizeOfImage				= pefile->size_of_image();
	pefile->inh->OptionalHeader.SizeOfInitializedData	+= exp_length + old_filesz;

	pefile->write(out_file.data());

	delete pefile;
}
