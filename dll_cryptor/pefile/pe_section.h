/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_SECTION_HEADER	sec_header;

class pe_section
{
public:
	pe_section(pe_base *base) : base(base) {};

	bool			is_bad(size_t id);
	size_t			get_count();

	sec_header		*get(size_t id);
	sec_header		*get(char *name);
	sec_header		*add(char *name, size_t size, size_t foffs, size_t flags = 0);

	char			*get_name(size_t id);
	char			*get_data(size_t id);
	char			*get_data(char *name);

	

private:
	pe_base			*base;
};

