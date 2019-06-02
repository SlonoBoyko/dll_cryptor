/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

pe_file::pe_file() : pe_base()
{
	exp = new pe_export	(this);
	imp = new pe_import	(this);
	rel = new pe_relocs	(this);
	tls = new pe_tls	(this);
	sec = new pe_section(this);
	res = new pe_rsrc	(this);
}

pe_file::pe_file(std::string filename) : pe_base(filename)
{
	exp = new pe_export	(this);
	imp = new pe_import	(this);
	rel = new pe_relocs	(this);
	tls = new pe_tls	(this);
	sec = new pe_section(this);
	res = new pe_rsrc	(this);
}

pe_file::~pe_file() 
{
	delete exp;
	delete imp;
	delete rel;
	delete tls;
	delete sec;
	delete res;
}













