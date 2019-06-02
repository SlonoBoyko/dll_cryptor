/* ========================== */
/*     © SlonoBoyko 2018      */
/* ========================== */

#include <main.h>

pe_base::pe_base() 
{
	data = new unsigned char[0x4];
	memset(data, NULL, 0x4);
};

pe_base::pe_base(std::string filename)
{
	if (read(filename)) load();
	else fclose(fopen(filename.data(), "wb"));
}

pe_base::~pe_base()
{
	if (data != nullptr)
	{
		delete[] data;
		fname	.clear();
		idh		= nullptr;
		inh		= nullptr;
		ish		= nullptr;
		size	= 0;
	}
}

bool pe_base::load()
{
	idh = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
	inh = reinterpret_cast<PIMAGE_NT_HEADERS>(data + idh->e_lfanew);
	ish = reinterpret_cast<PIMAGE_SECTION_HEADER>((uint8_t *)inh + sizeof(IMAGE_NT_HEADERS));
	idd = reinterpret_cast<PIMAGE_DATA_DIRECTORY>(inh->OptionalHeader.DataDirectory);

	if (idh->e_magic != IMAGE_DOS_SIGNATURE || inh->Signature != IMAGE_NT_SIGNATURE)
		return false;

	return true;
}

bool pe_base::read(std::string filename)
{
	if (_access(filename.data(), 0) == -1)
		return false;

	fname = filename;

	FILE *file = fopen(filename.data(), "rb");
	if (file == nullptr) return false;
	
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	data = new unsigned char[size];
	memset(data, NULL, size);
	fread(data, size, 1, file);
	fclose(file);

	return true;
}

bool pe_base::write(std::string filename)
{
	FILE *file = fopen(filename.data(), "wb");
	if (file == nullptr) return false;
	fwrite(data, size, 1, file);
	size_t align_val = inh->OptionalHeader.FileAlignment;
	while (ftell(file) < align(size, align_val)) fputc(0, file);

	fclose(file);

	return true;
}

size_t pe_base::rva_to_offset(size_t virtual_address)
{
	size_t address = virtual_address;
	for (size_t i = 0; i < inh->FileHeader.NumberOfSections; ++i) {
		size_t end_point = ish[i].VirtualAddress + ish[i].Misc.VirtualSize;
		size_t start_point = ish[i].VirtualAddress;
		if (address >= start_point && address < end_point)
			return (address - ish[i].VirtualAddress) + ish[i].PointerToRawData;
	} 
	return address;
}

size_t pe_base::offset_to_rva(size_t file_offset)
{
	size_t address = file_offset;
	for (size_t i = 0; i < inh->FileHeader.NumberOfSections; ++i) {
		size_t end_point	= ish[i].PointerToRawData + ish[i].SizeOfRawData;
		size_t start_point	= ish[i].PointerToRawData;
		if (address >= start_point && address < end_point)
			return (address + ish[i].VirtualAddress) - ish[i].PointerToRawData;
	} 
	return address;
}

size_t pe_base::size_of_image()
{
	size_t count = inh->FileHeader.NumberOfSections;
	size_t secal = inh->OptionalHeader.SectionAlignment;
	size_t vsize = align(ish[count - 1].SizeOfRawData, secal);
	return ish[count - 1].VirtualAddress + vsize;
}

bool pe_base::change_data(size_t off, void *ptr, size_t len)
{
	if (off >= size) return false;
	if (data == nullptr) return false;

	if (ptr == nullptr)
		memset(data + off, 0, len);
	else memmove(data + off, ptr, len);
	
	return true;
}

bool pe_base::insert_data(size_t off, void *ptr, size_t len)
{
	if (off >= size) return false;
	if (data == nullptr) return false;

	char *temp = new char[size + len];
	memmove(temp, data, off);
	
	if (ptr == nullptr)
		memset(temp + off, 0, len);
	else memmove(temp + off, ptr, len);
	
	memmove(temp + off + len, data + off, size - off);
	data = new unsigned char[size + len];
	memcpy(data, temp, size += len);
	delete[] temp;
	return true;
}

bool pe_base::append_data(void *ptr, size_t len)
{
	if (data == nullptr) return false;

	char *temp = new char[size + len];
	memmove(temp, data, size);
	if (ptr == nullptr)
		memset(data + size, 0, len);
	else memmove(temp + size, ptr, len);

	data = new unsigned char[size + len];
	memcpy(data, temp, size += len);	
	delete[] temp;
	return true;
}

size_t pe_base::align(size_t sz, size_t alignment)
{
	return ((sz % alignment) ? (sz + alignment - (sz % alignment)) : sz);
}