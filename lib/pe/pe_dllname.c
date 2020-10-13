#include "../pe.h"
char*
pe_dllname(uint8* base) {
  pe_data_directory* data_dir = &pe_header_datadir(base)[PE_DIRECTORY_ENTRY_EXPORT];
  pe_export_directory* export_dir = pe_rva2ptr(base, data_dir->virtual_address);
  return pe_rva2ptr(base, uint32_get(&export_dir->name));
}