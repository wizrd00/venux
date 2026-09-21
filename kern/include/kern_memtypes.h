#ifndef _KERN_MEMTYPES_H
#define _KERN_MEMTYPES_H

#define UEFI_BIOS 0
#define LEGACY_BIOS 1

#define UNKNOWN 0
#define AVAILABLE 1
#define RESERVED 2
#define ACPI_RECLAIM 3
#define ACPI_NVS 4

#define UEFI_RESERVED 0
#define UEFI_LOADER_CODE 1
#define UEFI_LOADER_DATA 2
#define UEFI_BOOT_SERVICES_CODE 3
#define UEFI_BOOT_SERVICES_DATA 4
#define UEFI_RUNTIME_SERVICES_CODE 5
#define UEFI_RUNTIME_SERVICES_DATA 6
#define UEFI_CONVENTIONAL 7
#define UEFI_UNUSABLE 8
#define UEFI_ACPI_RECLAIM 9
#define UEFI_ACPI_NVS 10
#define UEFI_MMIO 11
#define UEFI_MMIO_PORT_SPACE 12
#define UEFI_PAL_CODE 13
#define UEFI_PERSISTENT 14
#define UEFI_UNACCEPTED 15

static inline int
convert_memtype(int bios_type, int mem_type)
{
	switch (bios_type) {
	case UEFI_BIOS :
		switch (mem_type) {
		case UEFI_LOADER_CODE :
		case UEFI_BOOT_SERVICES_CODE :
		case UEFI_BOOT_SERVICES_DATA :
		case UEFI_CONVENTIONAL :
		case UEFI_PERSISTENT :
			return AVAILABLE;
		case UEFI_RESERVED :
		case UEFI_LOADER_DATA :
		case UEFI_RUNTIME_SERVICES_CODE :
		case UEFI_RUNTIME_SERVICES_DATA :
		case UEFI_UNUSABLE :
		case UEFI_MMIO :
		case UEFI_MMIO_PORT_SPACE :
		case UEFI_PAL_CODE :
		case UEFI_UNACCEPTED :
			return RESERVED;
		case UEFI_ACPI_RECLAIM :
			return ACPI_RECLAIM;
		case UEFI_ACPI_NVS :
			return ACPI_NVS;
		default :
			return UNKNOWN;
		}
	case LEGACY_BIOS :
		switch (mem_type) {
		case AVAILABLE :
			return AVAILABLE;
		case RESERVED :
			return RESERVED;
		case ACPI_RECLAIM :
			return ACPI_RECLAIM;
		case ACPI_NVS :
			return ACPI_NVS;
		default :
			return UNKNOWN;
		}
	default :
		return UNKNOWN;
	}
}

#endif
