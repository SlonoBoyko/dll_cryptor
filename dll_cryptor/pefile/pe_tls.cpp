/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include "main.h"

bool pe_tls::is_bad()
{
	return base->idd[IMAGE_DIRECTORY_ENTRY_TLS].Size == 0;
}

tls_dir	*pe_tls::directory()
{
	uint32_t tls_offset = base->rva_to_offset(base->idd[9].VirtualAddress);
	return reinterpret_cast<tls_dir *>(base->data + tls_offset);
}

size_t pe_tls::get_index(size_t id)
{
	uint32_t idx_offset = base->rva_to_offset(directory()->AddressOfIndex);
	return reinterpret_cast<size_t *>(base->data + idx_offset)[id];
}

tls_callback pe_tls::get_callback(size_t id)
{
	uint32_t callback_offset = base->rva_to_offset(directory()->AddressOfCallBacks);
	return reinterpret_cast<tls_callback *>(base->data + callback_offset)[id];
}

uintptr_t *pe_tls::get_start()
{
	uint32_t bgn_offset = base->rva_to_offset(directory()->StartAddressOfRawData);
	return reinterpret_cast<uintptr_t *>(base->data + bgn_offset);
}

uintptr_t *pe_tls::get_end()
{
	uint32_t end_offset = base->rva_to_offset(directory()->EndAddressOfRawData);
	return reinterpret_cast<uintptr_t *>(base->data + end_offset);
}
