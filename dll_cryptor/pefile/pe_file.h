/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

using namespace std;

class pe_export; 
class pe_import;
class pe_relocs;
class pe_tls;
class pe_section;
class pe_rsrc;

class pe_file : public pe_base
{
public:
						pe_file(std::string filename);
						pe_file();
						~pe_file();

	pe_export			*get_export	() { return exp; };
	pe_import			*get_import	() { return imp; };
	pe_relocs			*get_relocs	() { return rel; };
	pe_tls				*get_tls	() { return tls; };
	pe_section			*get_section() { return sec; };
	pe_rsrc				*get_rsrc	() { return res; };

private:
	pe_export			*exp;
	pe_import			*imp;
	pe_relocs			*rel;
	pe_tls				*tls;
	pe_section			*sec;
	pe_rsrc				*res;
};