/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_RESOURCE_DIRECTORY		rsrc_dir;
typedef IMAGE_RESOURCE_DIR_STRING_U		rsrc_str;
typedef IMAGE_RESOURCE_DATA_ENTRY		rsrc_data;
typedef IMAGE_RESOURCE_DIRECTORY_ENTRY	dir_entry;

class rsrc_entry
{
public:
	rsrc_entry(pe_base *base, dir_entry *data_ptr)
		: base(base), entry(data_ptr) {};

	bool			is_named();
	bool			is_directory();
	char			*get_name();
	size_t			get_id();

	size_t			get_count();
	rsrc_dir		*get_dir();
	rsrc_entry		*get_entry(size_t id);
	rsrc_data		*get_data();

private:
	dir_entry		*entry;
	pe_base			*base;
};

class pe_rsrc
{
public:
	pe_rsrc(pe_base *base) : base(base) {};

	bool			is_bad();
	size_t			get_count();
	rsrc_dir		*directory();
	rsrc_entry		*get_entry(size_t id);
	rsrc_data		*find_rsrc(size_t id);
	rsrc_data		*find_rsrc(char *name);

private:
	pe_base			*base;
};
