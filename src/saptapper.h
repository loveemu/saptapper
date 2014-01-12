
#ifndef SAPTAPPER_H_INCLUDED
#define SAPTAPPER_H_INCLUDED

#include <stdint.h>

#include "BytePattern.h"

#define INIT_COUNT 2

#define GSF_ROM_HEADER_SIZE	12
#define MAX_GBA_ROM_SIZE	0x2000000

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
		GSFLIB_ROM_WR,
		GSFLIB_OTFILE_E,
		GSFLIB_DIR_ERR,
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

	FILE *bat;

public:
	Saptapper() :
		manual(0),
		bat(NULL)
	{
		compbuf = new uint8_t[GSF_ROM_HEADER_SIZE + MAX_GBA_ROM_SIZE];
		uncompbuf = new uint8_t[GSF_ROM_HEADER_SIZE + MAX_GBA_ROM_SIZE];
	}

	~Saptapper()
	{
		delete[] compbuf;
		delete[] uncompbuf;
	}

	int isduplicate(int num);
	int doexe2gsf(unsigned long offset, int size, unsigned short num, const char *to, const char *base);
	EGsfLibResult dogsflib(const char *from, const char *to);
	int main(int argc, char **argv);

private:
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
