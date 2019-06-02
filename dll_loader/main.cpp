
#include "main.h"

#include <stdio.h>

/*void Log(const char *text, ...)
{
	va_list ap;
	if (text == NULL) return;

	char *log_file = "dll_cryptor.log";
	FILE* flog = fopen(log_file, "a");

	va_start(ap, text);
	vfprintf(flog, text, ap);
	va_end(ap);

	fprintf(flog, "\n");
	fclose(flog);
}*/

uint32_t module_base = 0;
uint32_t key[4] = { 0xACB5, 0x1344, 0xEC90, 0x285C };

void xtea_decode(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	unsigned int i;
	uint32_t v0 = v[0], v1 = v[1], delta = 0x9E3779B9, sum = delta * num_rounds;
	for (i = 0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0] = v0; v[1] = v1;
}

PVOID unpack_code(char *code, size_t size)
{
	int n_blocks = size / 8;
	if (size % 8 != 0) ++n_blocks;

	for (int i = 0; i < n_blocks; i++)
		xtea_decode(32, (unsigned int*)&code[i*8], key);

	return code;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwCall, LPVOID lpReserved)
{
	if (dwCall == DLL_PROCESS_ATTACH)
	{
		unpack_code(data, 0x80000);

		module_base = reinterpret_cast<uint32_t>(hModule);

		pe_load pe_file(data);
		pe_file.load_headers();
		pe_file.copy_section();
		pe_file.perform_relocs();
		pe_file.build_import();
		pe_file.build_export();
		pe_file.protect_flags();
		pe_file.call_entry_point();
	}
	return TRUE;
}