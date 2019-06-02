/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_EXPORT_DIRECTORY export_dir;

class pe_export
{
public:
	pe_export(pe_base *base) : base(base) {};

	bool			is_bad();

	export_dir		*directory();

	uint32_t		*get_funcs_ptr();
	uint32_t		*get_names_ptr();
	uint16_t		*get_ordinals_ptr();
	char			*get_name_ptr();

	size_t			get_func_address(size_t id);
	char			*get_func_name(size_t id);
	
	size_t			get_func_count();
	size_t			get_name_count();

	void			create_dir(char *lib_name, size_t func_addr, char *func_name);
	void			add_function(char *name, size_t address);

private:
	pe_base			*base;
};
