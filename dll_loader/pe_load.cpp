
#include "main.h"

pe_load::pe_load(char *lib) : data(reinterpret_cast<uint8_t *>(lib))
{
	idh = reinterpret_cast<dos_header*>	(data);
	inh = reinterpret_cast<nt_header*>	(data + idh->e_lfanew);

	base = inh->OptionalHeader.ImageBase;
	SYSTEM_INFO sysInfo; GetNativeSystemInfo(&sysInfo);
	size_t img_size = (inh->OptionalHeader.SizeOfImage + sysInfo.dwPageSize) - 1 & ~(sysInfo.dwPageSize - 1);
	code = (uint8_t *)VirtualAlloc((uint8_t *)base, img_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (code == nullptr) code = (uint8_t *)VirtualAlloc(NULL, img_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	head = (uint8_t *)VirtualAlloc(code, inh->OptionalHeader.SizeOfHeaders, MEM_COMMIT, PAGE_READWRITE);
}

bool pe_load::load_headers()
{
	if (idh->e_magic != IMAGE_DOS_SIGNATURE) return false;
	if (inh->Signature != IMAGE_NT_SIGNATURE) return false;
	if (inh->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return false;

	memcpy(head, idh, inh->OptionalHeader.SizeOfHeaders);

	idh = reinterpret_cast<dos_header*>	(head);
	inh = reinterpret_cast<nt_header*>	(head + idh->e_lfanew);
	ish = reinterpret_cast<sec_header*>	((byte *)inh + sizeof(IMAGE_NT_HEADERS));
	idd = reinterpret_cast<data_dir*>	(inh->OptionalHeader.DataDirectory);	
	inh->OptionalHeader.ImageBase		= reinterpret_cast<uintptr_t>(code);

	return true;
}

bool pe_load::protect_flags()
{
	bool result = false;
	for (size_t i = 0; i < inh->FileHeader.NumberOfSections; i++)
	{
		size_t protect; DWORD old;
		size_t chars = ish[i].Characteristics;
		if (chars & IMAGE_SCN_MEM_EXECUTE) {
			if(chars & IMAGE_SCN_MEM_READ)
				protect = (chars & IMAGE_SCN_MEM_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
			else result = (chars & IMAGE_SCN_MEM_WRITE) ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE;
		} else {
			if (chars & IMAGE_SCN_MEM_READ)
				protect = (chars & IMAGE_SCN_MEM_WRITE) ? PAGE_READWRITE : PAGE_READONLY;
			else result = (chars & IMAGE_SCN_MEM_WRITE) ? PAGE_WRITECOPY : PAGE_NOACCESS;
		}
		if (chars & IMAGE_SCN_MEM_NOT_CACHED) protect |= PAGE_NOCACHE;
	
		uint8_t *dest = code + ish[i].VirtualAddress;
		result = VirtualProtect(dest, ish[i].Misc.VirtualSize, protect, &old);
	}
	return result;
}

bool pe_load::copy_section()
{
	idd[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = 0;
	idd[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size = 0;
	idd[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
	idd[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
	idd[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;
	idd[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = 0;

	for (size_t i = 0; i < inh->FileHeader.NumberOfSections; i++)
	{
		uint8_t *dest = nullptr;
		size_t sec_size = ish[i].SizeOfRawData > 0 ? ish[i].SizeOfRawData : inh->OptionalHeader.SectionAlignment;
		dest = (uint8_t *)VirtualAlloc(code + ish[i].VirtualAddress, sec_size, MEM_COMMIT, PAGE_READWRITE);
		if (dest == nullptr) return false;
		if (ish[i].SizeOfRawData > 0) {
			dest = code + ish[i].VirtualAddress;

			memcpy(dest, data + ish[i].PointerToRawData, ish[i].SizeOfRawData);
		}
	}
	return true;
}

bool pe_load::perform_relocs()
{
	ptrdiff_t delta = static_cast<ptrdiff_t>(inh->OptionalHeader.ImageBase - base);
	reloc_dir *reloc = reinterpret_cast<reloc_dir*>(code + idd[5].VirtualAddress);
	
	while (delta != 0 && reloc != nullptr && reloc->SizeOfBlock)
	{
		size_t count = (reloc->SizeOfBlock - sizeof(reloc_dir)) / sizeof(uint16_t);
		uint16_t *rdata = reinterpret_cast<uint16_t *>((uint8_t*)reloc + sizeof(reloc_dir));
		
		for (size_t i = 0; i < count; i++) 
		{
			if ((rdata[i] >> 12) == IMAGE_REL_BASED_HIGHLOW) {
				uint32_t address = reinterpret_cast<uint32_t>(code + reloc->VirtualAddress);
				*reinterpret_cast<uint32_t *>(address + (rdata[i] & 0xfff)) += delta;
			}
		} 
		reloc = reinterpret_cast<reloc_dir*>((uint8_t*)reloc + reloc->SizeOfBlock);
	} 
	return true;
}

bool pe_load::build_import()
{
	import_dir *deskriptor = reinterpret_cast<import_dir *>(code + idd[1].VirtualAddress);
	if (idd[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0) return TRUE;

	for (; deskriptor->Name != NULL; deskriptor++)
	{	
		HMODULE handle = LoadLibrary((LPCSTR)(code + deskriptor->Name));
		if (handle == NULL) break;

		uintptr_t	*ptr_func	= reinterpret_cast<uintptr_t *>(code + deskriptor->FirstThunk);
		thunk_data	*ptr_thunk	= reinterpret_cast<thunk_data *>(code + deskriptor->FirstThunk);

		if (deskriptor->OriginalFirstThunk != NULL) {
			ptr_thunk = reinterpret_cast<thunk_data *>(code + deskriptor->OriginalFirstThunk);
		}
		for (size_t i = 0; ptr_thunk[i].u1.Ordinal; i++) {
			if (IMAGE_SNAP_BY_ORDINAL(ptr_thunk[i].u1.Ordinal)) {
				ptr_func[i] = (uintptr_t)GetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(ptr_thunk[i].u1.Ordinal));
			} else {
				import_name *thunk = reinterpret_cast<import_name*>(code + ptr_thunk[i].u1.ForwarderString);
				ptr_func[i] = (uintptr_t)GetProcAddress(handle, reinterpret_cast<char*>(&thunk->Name));
			}

		}
	}
	return true;
}

bool pe_load::call_entry_point()
{
	HINSTANCE handle = reinterpret_cast<HINSTANCE>(code);
	uintptr_t entry_point = reinterpret_cast<uintptr_t>(code + inh->OptionalHeader.AddressOfEntryPoint);
	return reinterpret_cast<BOOL(__stdcall *)(HINSTANCE, DWORD, LPVOID)>(entry_point)(handle, DLL_PROCESS_ATTACH, 0);
}

uint32_t pe_load::get_proc_addr(char *name)
{
	uint32_t idx = 0;
	if (idd[0].Size == 0) return NULL;
	export_dir *exports = reinterpret_cast<export_dir*>(code + idd[0].VirtualAddress);
	if (exports->NumberOfNames == 0 || exports->NumberOfFunctions == 0)	return NULL;

	uint32_t *nameRef = reinterpret_cast<uint32_t *>(code + exports->AddressOfNames);
	uint16_t *ordinal = reinterpret_cast<uint16_t *>(code + exports->AddressOfNameOrdinals);
	for (size_t i = 0; i < exports->NumberOfNames; i++, nameRef++, ordinal++) {
		const char *nbuf = reinterpret_cast<char *>(code + *nameRef);
		if (!strcmp(nbuf, name)) { idx = *ordinal; break; }
	}
	if (idx > exports->NumberOfFunctions) return NULL;
	uint32_t *funcRef = reinterpret_cast<uint32_t *>(code + exports->AddressOfFunctions);
	return reinterpret_cast<uint32_t>(code + funcRef[idx]);
	return 0;
}

bool pe_load::build_export()
{
	uint32_t	base		= module_base;
	dos_header	*dos_head	= reinterpret_cast<dos_header*>	(base);
	nt_header	*nt_head	= reinterpret_cast<nt_header*>	(base + dos_head->e_lfanew);
	data_dir	*dir		= reinterpret_cast<data_dir*>	(nt_head->OptionalHeader.DataDirectory);

	export_dir	*new_exp	= reinterpret_cast<export_dir*>	(base + dir[0].VirtualAddress);
	export_dir	*old_exp	= reinterpret_cast<export_dir*>	(code + idd[0].VirtualAddress);
	
	uint32_t	*new_func	= reinterpret_cast<uint32_t *>	(base + new_exp->AddressOfFunctions);
	uint32_t	*old_func	= reinterpret_cast<uint32_t *>	(code + old_exp->AddressOfFunctions);

	DWORD old_protect;
	
	for (size_t i = 0; i < new_exp->NumberOfFunctions; i++)
	{
		VirtualProtect(&new_func[i], 4, PAGE_EXECUTE_READWRITE, &old_protect);
		new_func[i] = ((uint32_t)code + old_func[i]) - base;
		VirtualProtect(&new_func[i], 4, old_protect, &old_protect);
	}

	return true;
}