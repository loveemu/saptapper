
#ifndef SAPTAPPER_H_INCLUDED
#define SAPTAPPER_H_INCLUDED

#include <stdint.h>
#include <string>
#include <map>

#include "BytePattern.h"

#define INIT_COUNT	2

#define PSF_SIGNATURE	"PSF"
#define PSF_TAG_SIGNATURE	"[TAG]"

#define GSF_PSF_VERSION	0x22
#define GSF_EXE_HEADER_SIZE	12

#define MAX_GBA_ROM_SIZE	0x2000000
#define MAX_GSF_EXE_SIZE	(MAX_GBA_ROM_SIZE + GSF_EXE_HEADER_SIZE)

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

public:
	uint8_t* compbuf;
	uint8_t* uncompbuf;

	static const uint8_t selectsong[0x1E];
	static const uint8_t init[2][INIT_COUNT];
	static const uint8_t soundmain[2];
	static const uint8_t vsync[5];
	static const uint8_t SAPPYBLOCK[248];

	uint32_t minigsfoffset;
	uint32_t minigsfcount;

	uint32_t entrypoint;
	uint32_t load_offset;
	uint32_t rom_size;

	uint32_t sappyoffset;
	int manual;

public:
	Saptapper() :
		manual(0)
	{
		compbuf = new uint8_t[MAX_GSF_EXE_SIZE];
		uncompbuf = new uint8_t[MAX_GSF_EXE_SIZE];
	}

	~Saptapper()
	{
	}

	static void put_gsf_exe_header(uint8_t *exe, uint32_t entrypoint, uint32_t load_offset, uint32_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size);
	static bool exe2gsf(const std::string& gsf_path, uint8_t *rom, size_t rom_size, std::map<std::string, std::string>& tags);
	static bool make_minigsf(const std::string& gsf_path, uint32_t offset, size_t size, uint32_t num, std::map<std::string, std::string>& tags);

	static const char* get_gsflib_error(EGsfLibResult error_type);
	EGsfLibResult dogsflib(const char *from, const char *to);

	bool make_gsf_set(const std::string& rom_path);

private:
	int isduplicate(uint8_t *rom, uint32_t sappyoffset, int num);

	inline bool is_gba_rom_address(uint32_t address)
	{
		uint8_t region = (address >> 24) & 0xFE;
		return (region == 8);
	}

	inline uint32_t gba_address_to_offset(uint32_t address)
	{
		if (!is_gba_rom_address(address)) {
			//fprintf(stderr, "Warning: the address $%08X is not ROM address\n", address);
		}
		return address & 0x01FFFFFF;
	}
};

#endif
