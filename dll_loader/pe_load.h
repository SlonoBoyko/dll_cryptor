#pragma once
class pe_load
{
public:
	using dos_header	= IMAGE_DOS_HEADER;
	using nt_header		= IMAGE_NT_HEADERS;
	using sec_header	= IMAGE_SECTION_HEADER;
	using data_dir		= IMAGE_DATA_DIRECTORY;
	using reloc_dir		= IMAGE_BASE_RELOCATION;
	using import_dir	= IMAGE_IMPORT_DESCRIPTOR;
	using export_dir	= IMAGE_EXPORT_DIRECTORY;
	using thunk_data	= IMAGE_THUNK_DATA;
	using import_name	= IMAGE_IMPORT_BY_NAME;

						pe_load(char *lib);
	bool				load_headers();
	bool				copy_section();
	bool				perform_relocs();
	bool				build_import();
	bool				protect_flags();
	bool				call_entry_point();
	bool				build_export();

	uint32_t 			get_proc_addr(char *name);

	

//private:
	dos_header			*idh;
	nt_header			*inh;
	sec_header			*ish;
	data_dir			*idd;
	uint8_t				*data;
	uint8_t				*head;
	uint8_t				*code;
	size_t				base;
};
