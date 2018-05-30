
#ifndef SAPTAPPER_H_INCLUDED
#define SAPTAPPER_H_INCLUDED

#include <stdint.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>

#include "VgmDriver.h"
#include "Mp2kDriver.h"
#include "NatsumeDriver.h"
#include "BytePattern.h"

#define PSF_SIGNATURE	"PSF"
#define PSF_TAG_SIGNATURE	"[TAG]"

#define GBA_ENTRYPOINT	0x08000000

#define GSF_PSF_VERSION	0x22
#define GSF_EXE_HEADER_SIZE	12

#define MAX_GBA_ROM_SIZE	0x02000000
#define MAX_GSF_EXE_SIZE	(MAX_GBA_ROM_SIZE + GSF_EXE_HEADER_SIZE)

#define GSF_INVALID_OFFSET	0xFFFFFFFF

static inline bool is_gba_rom_address(uint32_t address)
{
	uint8_t region = (address >> 24) & 0xFE;
	return (region == 8);
}

static inline uint32_t gba_address_to_offset(uint32_t address)
{
	if (!is_gba_rom_address(address)) {
		//fprintf(stderr, "Warning: the address $%08X is not ROM address\n", address);
	}
	return address & 0x01FFFFFF;
}

static inline uint32_t gba_offset_to_address(uint32_t offset)
{
	return 0x08000000 | (offset & 0x01FFFFFF);
}

static inline uint32_t make_arm_branch(uint32_t current_address, uint32_t destination_address)
{
	assert(current_address % 4 == 0 && destination_address % 4 == 0);
	int32_t offset = (destination_address - (current_address + 8)) / 4;
	return 0xEA000000 | (offset & 0xFFFFFF);
}

static inline bool is_arm_branch(uint32_t instruction)
{
	return (instruction & 0xFF000000) == 0xEA000000;
}

static inline uint32_t get_arm_branch_destination(uint32_t current_address, uint32_t instruction)
{
	if ((instruction & 0x0A000000) != 0x0A000000) {
		return 0;
	}

	int32_t offset = (instruction & 0xFFFFFF);
	if ((offset & 0x800000) != 0) {
		offset |= ~0xFFFFFF;
	}

	return current_address + 8 + (offset * 4);
}

static inline bool is_thumb_branch_with_link(uint32_t instruction)
{
	return (instruction & 0xF800F800) == 0xF800F000;
}

static inline uint32_t get_thumb_branch_with_link_destination(uint32_t current_address, uint32_t instruction)
{
	int32_t offset = ((instruction & 0x7FF) << 12) | ((instruction & 0x7FF0000) >> 16 << 1);
	if ((offset & 0x400000) != 0) {
		offset |= ~0x7FFFFF;
	}

	return current_address + 4 + offset;
}

static inline void fill_with_nop_arm(uint8_t * rom, size_t size)
{
	size &= ~3; // 4 bytes alignment

	for (size_t offset = 0; offset < size; offset += 4) {
		rom[offset] = 0x00;
		rom[offset + 1] = 0x00;
		rom[offset + 2] = 0xa0;
		rom[offset + 3] = 0xe1;
	}
}

static inline void fill_with_nop_thumb(uint8_t * rom, size_t size)
{
	size &= ~1; // 2 bytes alignment

	for (size_t offset = 0; offset < size; offset += 2) {
		rom[offset] = 0xc0;
		rom[offset + 1] = 0x46;
	}
}

class Saptapper
{
private:
	uint8_t* rom;
	uint8_t* rom_exe;
	size_t rom_size;
	uint32_t rom_main_ptr_offset;

	std::vector<VgmDriver*> drivers;

	uint8_t* rom_backup;
	size_t rom_backup_size;

	uint8_t* manual_gsf_driver;
	uint32_t manual_gsf_driver_offset;
	size_t manual_gsf_driver_size;
	uint32_t manual_minigsf_offset;
	uint32_t manual_minigsf_count;
	bool manual_driver_is_thumb;
	bool manual_driver_is_main;

	std::string tag_gsfby;

	bool quiet;
	bool prefer_larger_free_space;

public:
	Saptapper() :
		rom(NULL),
		rom_exe(NULL),
		rom_size(0),
		rom_main_ptr_offset(GSF_INVALID_OFFSET),
		rom_backup(NULL),
		rom_backup_size(0),
		manual_gsf_driver(NULL),
		manual_gsf_driver_offset(GSF_INVALID_OFFSET),
		manual_gsf_driver_size(0),
		manual_minigsf_offset(GSF_INVALID_OFFSET),
		manual_minigsf_count(GSF_INVALID_OFFSET),
		manual_driver_is_thumb(false),
		manual_driver_is_main(false),
		quiet(true),
		prefer_larger_free_space(false)
	{
		drivers.push_back(new Mp2kDriver());
		drivers.push_back(new NatsumeDriver());
	}

	~Saptapper()
	{
		for (std::vector<VgmDriver*>::iterator it = drivers.begin() ; it != drivers.end(); ++it)
		{
			delete (*it);
		}

		unset_gsf_driver();
		close_rom();
	}

	static void put_gsf_exe_header(uint8_t *exe, uint32_t entrypoint, uint32_t load_offset, uint32_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size, const std::map<std::string, std::string>& tags);
	static bool make_minigsf(const std::string& gsf_path, uint32_t address, size_t size, uint32_t num, std::map<std::string, std::string>& tags);

	inline void set_gsf_driver_offset(uint32_t offset)
	{
		manual_gsf_driver_offset = offset;
	}

	inline void set_minigsf_count(uint32_t count)
	{
		manual_minigsf_count = count;
	}

	inline void set_manual_driver_is_main(bool use_main)
	{
		manual_driver_is_main = use_main;
	}

	bool set_gsf_driver(uint8_t* driver_block, size_t size, uint32_t minigsf_offset, bool thumb, bool use_main);
	bool set_gsf_driver_file(const std::string& driver_path, uint32_t minigsf_offset, bool thumb, bool use_main);
	void unset_gsf_driver(void);

	bool load_rom(uint8_t* rom, size_t rom_size);
	bool load_rom_file(const std::string& rom_path);
	void scan_main_ptr(void);
	void close_rom(void);

	uint32_t find_free_space(size_t size, uint8_t filler);
	uint32_t find_free_space(size_t size);

	VgmDriver * make_gsflib(const std::string& gsf_path, bool prefer_gba_rom);
	VgmDriver * make_gsflib(const std::string& gsf_path, bool prefer_gba_rom, std::map<std::string, VgmDriverParam>& driver_params, uint32_t * ptr_driver_offset);
	bool make_gsf_set(const std::string& rom_path, bool prefer_gba_rom);
	bool make_gsf_set(const std::string& rom_path, bool prefer_gba_rom, std::map<std::string, VgmDriverParam>& driver_params);

	inline void set_tag_gsfby(const std::string& gsfby)
	{
		this->tag_gsfby = gsfby;
	}

	inline void set_quiet(bool quiet)
	{
		this->quiet = quiet;
	}

	inline void set_prefer_larger_free_space(bool prefer_larger_free_space)
	{
		this->prefer_larger_free_space = prefer_larger_free_space;
	}

	inline std::string message()
	{
		return m_message;
	}

private:
	void create_rom_backup(const uint8_t * rom, size_t rom_size);
	void restore_rom_from_backup(uint8_t * rom, size_t rom_size);
	void destroy_rom_backup(uint8_t * & rom_backup, size_t & rom_backup_size);
	bool install_driver(const uint8_t* driver_block, uint32_t offset, size_t size, bool thumb, bool use_main);
	void uninstall_driver(void);

	void print_driver_params(const std::map<std::string, VgmDriverParam>& driver_params) const;

	std::string m_message;
};

#endif
