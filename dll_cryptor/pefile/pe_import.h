/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_IMPORT_DESCRIPTOR import_dir;
typedef IMAGE_THUNK_DATA		import_thunk;

class import_data
{
public:
	import_data(pe_base	*base, uint32_t *data_ptr) 
		: base(base), thunk(data_ptr) {};

	bool		is_by_name();
	size_t		get_count();
	size_t		get_ordinal(size_t id);
	uint16_t	get_index(size_t id);
	char		*get_name(size_t id);

private:
	uint32_t		*thunk;
	pe_base			*base;
}; 

class pe_import
{
public:
	pe_import(pe_base *base) : base(base) {};

	bool			is_bad();

	import_dir		*descriptor(size_t id);

	size_t			get_library_count();
	char			*get_library_name(size_t id);
	size_t			get_func_count(size_t id);
	import_thunk	*get_first_thunk(size_t id);
	import_data		*get_data(size_t id);
	

private:
	pe_base			*base;
};



