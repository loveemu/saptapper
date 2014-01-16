
#ifndef SAPTAPPER_H_INCLUDED
#define SAPTAPPER_H_INCLUDED

#include <stdint.h>
#include <string>
#include <map>

#include "BytePattern.h"

#define PSF_SIGNATURE	"PSF"
#define PSF_TAG_SIGNATURE	"[TAG]"

#define GBA_ENTRYPOINT	0x08000000

#define GSF_PSF_VERSION	0x22
#define GSF_EXE_HEADER_SIZE	12

#define MAX_GBA_ROM_SIZE	0x02000000
#define MAX_GSF_EXE_SIZE	(MAX_GBA_ROM_SIZE + GSF_EXE_HEADER_SIZE)

#define GSF_INVALID_OFFSET	0xFFFFFFFF

class Saptapper
{
public:
	enum EGsfLibResult
	{
		GSFLIB_OK = 0,
		GSFLIB_NOMAIN,
		GSFLIB_NOSELECT,
		GSFLIB_NOINIT,
		GSFLIB_NOVSYNC,
		GSFLIB_NOSPACE,
		GSFLIB_ZLIB_ERR,
		GSFLIB_INFILE_E,
		GSFLIB_OTFILE_E,
	};

private:
	uint8_t* rom;
	uint8_t* rom_exe;
	size_t rom_size;

	// create backup of driver location
	uint32_t rom_patch_entrypoint_backup;	// ARM B _start
	uint8_t* rom_patch_backup;
	uint32_t rom_patch_offset;
	size_t rom_patch_size;

	uint32_t offset_m4a_main;
	uint32_t offset_m4a_selectsong;
	uint32_t offset_m4a_init;
	uint32_t offset_m4a_vsync;
	uint32_t offset_m4a_songtable;
	uint32_t offset_gsf_driver;
	uint32_t offset_minigsf_number;

	uint32_t manual_offset_m4a_main;
	uint32_t manual_offset_m4a_selectsong;
	uint32_t manual_offset_m4a_init;
	uint32_t manual_offset_m4a_vsync;
	uint32_t manual_offset_m4a_songtable;
	uint32_t manual_offset_gsf_driver;

	std::string tag_gsfby;

	bool quiet;
	bool prefer_larger_free_space;

public:
	Saptapper() :
		rom(NULL),
		rom_exe(NULL),
		rom_size(0),
		rom_patch_entrypoint_backup(0),
		rom_patch_backup(NULL),
		rom_patch_offset(GSF_INVALID_OFFSET),
		rom_patch_size(0),
		offset_m4a_main(GSF_INVALID_OFFSET),
		offset_m4a_selectsong(GSF_INVALID_OFFSET),
		offset_m4a_init(GSF_INVALID_OFFSET),
		offset_m4a_vsync(GSF_INVALID_OFFSET),
		offset_m4a_songtable(GSF_INVALID_OFFSET),
		offset_gsf_driver(GSF_INVALID_OFFSET),
		offset_minigsf_number(GSF_INVALID_OFFSET),
		manual_offset_m4a_main(GSF_INVALID_OFFSET),
		manual_offset_m4a_selectsong(GSF_INVALID_OFFSET),
		manual_offset_m4a_init(GSF_INVALID_OFFSET),
		manual_offset_m4a_vsync(GSF_INVALID_OFFSET),
		manual_offset_m4a_songtable(GSF_INVALID_OFFSET),
		manual_offset_gsf_driver(GSF_INVALID_OFFSET),
		quiet(true),
		prefer_larger_free_space(false)
	{
	}

	~Saptapper()
	{
		close_rom();
	}

	static void put_gsf_exe_header(uint8_t *exe, uint32_t entrypoint, uint32_t load_offset, uint32_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size, std::map<std::string, std::string>& tags);
	static bool make_minigsf(const std::string& gsf_path, uint32_t address, size_t size, uint32_t num, std::map<std::string, std::string>& tags);
	static const char* get_gsflib_error(EGsfLibResult error_type);

	inline void set_m4a_main(uint32_t offset)
	{
		manual_offset_m4a_main = offset;
	}

	inline void set_m4a_selectsong(uint32_t offset)
	{
		manual_offset_m4a_selectsong = offset;
	}

	inline void set_m4a_init(uint32_t offset)
	{
		manual_offset_m4a_init = offset;
	}

	inline void set_m4a_vsync(uint32_t offset)
	{
		manual_offset_m4a_vsync = offset;
	}

	inline void set_m4a_songtable(uint32_t offset)
	{
		manual_offset_m4a_songtable = offset;
	}

	inline void set_gsf_driver_offset(uint32_t offset)
	{
		manual_offset_gsf_driver = offset;
	}

	bool load_rom(uint8_t* rom, size_t rom_size);
	bool load_rom_file(const std::string& rom_path);
	void close_rom(void);

	uint32_t find_m4a_selectsong(void);
	uint32_t find_m4a_songtable(uint32_t offset_m4a_selectsong);
	uint32_t find_m4a_main(uint32_t offset_m4a_selectsong);
	uint32_t find_m4a_init(uint32_t offset_m4a_main);
	uint32_t find_m4a_vsync(uint32_t offset_m4a_init);
	uint32_t find_free_space(size_t size, uint8_t filler);
	uint32_t find_free_space(size_t size);
	EGsfLibResult find_m4a_addresses(void);
	unsigned int get_song_count(uint32_t offset_m4a_songtable);

	EGsfLibResult make_gsflib(const std::string& gsf_path);
	bool make_gsf_set(const std::string& rom_path);

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

private:
	bool install_driver(uint8_t* driver_block, uint32_t offset, size_t size);
	void uninstall_driver(void);

	bool is_song_duplicate(uint32_t offset_m4a_songtable, unsigned int song_index);
};

#endif
