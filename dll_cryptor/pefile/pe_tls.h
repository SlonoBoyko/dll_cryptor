/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#pragma once

typedef IMAGE_TLS_DIRECTORY tls_dir;
typedef PIMAGE_TLS_CALLBACK tls_callback;

class pe_tls
{
public:
	pe_tls(pe_base *base) : base(base) {};
	
	bool			is_bad();
	tls_dir			*directory();
	uintptr_t		*get_start();
	uintptr_t		*get_end();
	
	size_t			get_index(size_t id);
	tls_callback	get_callback(size_t id);

private:
	pe_base			*base;
};

