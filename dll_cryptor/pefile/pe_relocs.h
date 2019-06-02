/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_BASE_RELOCATION reloc_dir;

class reloc_data
{
public:
	reloc_data(pe_base* base, reloc_dir *data_ptr) 
		: base(base), reloc(data_ptr) {};

	size_t			get_address();
	size_t			get_count();
	uint16_t		get_offset(size_t id);
	uint8_t			get_type(size_t id);

private:
	reloc_dir		*reloc;
	pe_base			*base;
};

class pe_relocs
{
public:
	pe_relocs(pe_base *base) : base(base) {};

	bool			is_bad();
	reloc_dir		*directory(size_t id);
	reloc_data		*get_block(size_t id);
	size_t			blocks_count();
	bool			add_reloc(size_t block_addr, uint16_t offs, uint8_t type = 3);

private:
	pe_base			*base;
};

