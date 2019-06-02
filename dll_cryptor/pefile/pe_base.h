/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

//#define Align(sz, alignment) (((sz) % (alignment)) ? ((sz)+(alignment)-((sz) % (alignment))) : (sz))


class pe_base 
{
public:
								pe_base(std::string filename);
								pe_base();
								~pe_base();

	bool						load();
	bool						read(std::string filename);
	bool						write(std::string filename);

	size_t						offset_to_rva(size_t file_offset);
	size_t						rva_to_offset(size_t virtual_address);
	size_t						align(size_t sz, size_t alignment);
	size_t						size_of_image();	

	template<typename T> bool	change_data(size_t off, T val) { return change_data(off, &val, sizeof(T)); };
	bool						change_data(size_t off, void *ptr, size_t len);
	bool						insert_data(size_t off, void *ptr, size_t len);
	bool						append_data(void *ptr, size_t len);
	
	IMAGE_DOS_HEADER			*idh = nullptr;
	IMAGE_NT_HEADERS			*inh = nullptr;
	IMAGE_SECTION_HEADER		*ish = nullptr;
	IMAGE_DATA_DIRECTORY		*idd = nullptr;

	unsigned char				*data = nullptr;
	std::string					fname;
	size_t						size = 0;
};
