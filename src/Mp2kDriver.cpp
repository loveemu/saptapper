
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <string>
#include <map>
#include <stdexcept>

#include "Mp2kDriver.h"
#include "BytePattern.h"
#include "cbyteio.h"
#include "saptapper.h"

#include "asm/sappy-gsf.h"

static unsigned int memcmploose(const void *buf1, const void *buf2, size_t n, unsigned int maxdiff)
{
	size_t i;
	int diff = 0;

	if (maxdiff == 0)
	{
		return 0;
	}

	for (i = 0; i < n; i++)
	{
		if (((uint8_t*)buf1)[i] != ((uint8_t*)buf2)[i])
		{
			diff++;

			if (diff == maxdiff)
			{
				break;
			}
		}
	}
	return diff;
}

Mp2kDriver::Mp2kDriver() :
	m_name("MP2000"),
	m_version("1.0")
{
}

Mp2kDriver::~Mp2kDriver()
{
}

// Return human-friendly driver name.
const std::string& Mp2kDriver::GetName() const
{
	return m_name;
}

// Return class version.
const std::string& Mp2kDriver::GetVersion() const
{
	return m_version;
}

// Parse ROM and collect necessary parameters to construct driver block.
void Mp2kDriver::FindDriverParams(const uint8_t * rom, size_t rom_size, std::map<std::string, VgmDriverParam>& params)
{
	uint32_t sub_selectsong = GSF_INVALID_OFFSET;
	if (params.count("sub_selectsong") == 0)
	{
		sub_selectsong = find_m4a_selectsong(rom, rom_size);
		if (sub_selectsong == GSF_INVALID_OFFSET)
		{
			return;
		}
		params["sub_selectsong"] = VgmDriverParam(gba_offset_to_address(sub_selectsong), true);
	}
	else
	{
		sub_selectsong = params.at("sub_selectsong").getInteger();
		if (sub_selectsong == 0)
		{
			sub_selectsong = GSF_INVALID_OFFSET;
		}
		else if (sub_selectsong >= 0x8000000 && sub_selectsong < 0x8000000 + rom_size)
		{
			sub_selectsong = gba_address_to_offset(sub_selectsong);
		}
		else
		{
			return;
		}
	}

	uint32_t array_songs = GSF_INVALID_OFFSET;
	if (params.count("_array_songs") == 0)
	{
		array_songs = find_m4a_songtable(rom, rom_size, sub_selectsong);
		if (array_songs != GSF_INVALID_OFFSET)
		{
			params["_array_songs"] = VgmDriverParam(gba_offset_to_address(array_songs), true);
		}
	}
	else
	{
		array_songs = params.at("_array_songs").getInteger();
		if (array_songs >= 0x8000000 && array_songs < 0x8000000 + rom_size)
		{
			array_songs = gba_address_to_offset(array_songs);
		}
		else
		{
			array_songs = GSF_INVALID_OFFSET;
		}
	}

	uint32_t sub_main = GSF_INVALID_OFFSET;
	if (params.count("sub_main") == 0)
	{
		sub_main = find_m4a_main(rom, rom_size, sub_selectsong);
		if (sub_main == GSF_INVALID_OFFSET)
		{
			return;
		}
		params["sub_main"] = VgmDriverParam(gba_offset_to_address(sub_main), true);
	}
	else
	{
		sub_main = params.at("sub_main").getInteger();
		if (sub_main == 0)
		{
			sub_main = GSF_INVALID_OFFSET;
		}
		else if (sub_main >= 0x8000000 && sub_main < 0x8000000 + rom_size)
		{
			sub_main = gba_address_to_offset(sub_main);
		}
		else
		{
			return;
		}
	}

	uint32_t sub_init = GSF_INVALID_OFFSET;
	if (params.count("sub_init") == 0)
	{
		sub_init = find_m4a_init(rom, rom_size, sub_main);
		if (sub_init == GSF_INVALID_OFFSET)
		{
			return;
		}
		params["sub_init"] = VgmDriverParam(gba_offset_to_address(sub_init), true);
	}
	else
	{
		sub_init = params.at("sub_init").getInteger();
		if (sub_init == 0)
		{
			sub_init = GSF_INVALID_OFFSET;
		}
		else if (sub_init >= 0x8000000 && sub_init < 0x8000000 + rom_size)
		{
			sub_init = gba_address_to_offset(sub_init);
		}
		else
		{
			return;
		}
	}

	uint32_t sub_vsync = GSF_INVALID_OFFSET;
	if (params.count("sub_vsync") == 0)
	{
		sub_vsync = find_m4a_vsync(rom, rom_size, sub_init);
		if (sub_vsync == GSF_INVALID_OFFSET)
		{
			return;
		}
		params["sub_vsync"] = VgmDriverParam(gba_offset_to_address(sub_vsync), true);
	}
	else
	{
		sub_vsync = params.at("sub_vsync").getInteger();
		if (sub_vsync == 0)
		{
			sub_vsync = GSF_INVALID_OFFSET;
		}
		else if (sub_vsync >= 0x8000000 && sub_vsync < 0x8000000 + rom_size)
		{
			sub_vsync = gba_address_to_offset(sub_vsync);
		}
		else
		{
			return;
		}
	}
}

// Validate driver parameters and return if driver block is installable or not.
bool Mp2kDriver::ValidateDriverParams(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params)
{
	char str[512];

	if (params.count("sub_selectsong") == 0)
	{
		sprintf(str, "sappy_selectsong not found");
		m_message = str;
		return false;
	}
	uint32_t sub_selectsong = params.at("sub_selectsong").getInteger();
	if (sub_selectsong != 0 && (sub_selectsong < 0x08000000 || sub_selectsong >= 0x8000000 + rom_size))
	{
		sprintf(str, "sappy_selectsong is out of range (0x%08X)", sub_selectsong);
		m_message = str;
		return false;
	}

	if (params.count("sub_main") == 0)
	{
		sprintf(str, "sappy_main not found (sappy_selectsong=0x%08X)", sub_selectsong);
		m_message = str;
		return false;
	}
	uint32_t sub_main = params.at("sub_main").getInteger();
	if (sub_main != 0 && (sub_main < 0x08000000 || sub_main >= 0x8000000 + rom_size))
	{
		sprintf(str, "sappy_main is out of range (0x%08X)", sub_main);
		m_message = str;
		return false;
	}

	if (params.count("sub_init") == 0)
	{
		sprintf(str, "sappy_init not found (sappy_selectsong=0x%08X, sappy_main=0x%08X)", sub_selectsong, sub_main);
		m_message = str;
		return false;
	}
	uint32_t sub_init = params.at("sub_init").getInteger();
	if (sub_init != 0 && (sub_init < 0x08000000 || sub_init >= 0x8000000 + rom_size))
	{
		sprintf(str, "sappy_init is out of range (0x%08X)", sub_init);
		m_message = str;
		return false;
	}

	if (params.count("sub_vsync") == 0)
	{
		sprintf(str, "sappy_vsync not found (sappy_selectsong=0x%08X, sappy_main=0x%08X, sappy_init=0x%08X)", sub_selectsong, sub_main, sub_init);
		m_message = str;
		return false;
	}
	uint32_t sub_vsync = params.at("sub_vsync").getInteger();
	if (sub_vsync != 0 && (sub_vsync < 0x08000000 || sub_vsync >= 0x8000000 + rom_size))
	{
		sprintf(str, "sappy_vsync is out of range (0x%08X)", sub_vsync);
		m_message = str;
		return false;
	}

	return true;
}

// Return song count in the ROM. Return 0 if not obtainable.
int Mp2kDriver::GetSongCount(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params)
{
	if (params.count("_array_songs") == 0)
	{
		return 0;
	}
	uint32_t array_songs = params.at("_array_songs").getInteger();
	if (array_songs < 0x08000000 || array_songs >= 0x8000000 + rom_size)
	{
		return 0;
	}
	array_songs = gba_address_to_offset(array_songs);
	return m4a_get_song_count(rom, rom_size, array_songs);
}

// Return offset of song index variable in relocatable driver block.
off_t Mp2kDriver::GetMinixsfOffset(const std::map<std::string, VgmDriverParam>& params) const
{
	return 0xE8;
}

// Return relocatable driver block size.
size_t Mp2kDriver::GetDriverSize(const std::map<std::string, VgmDriverParam>& params) const
{
	return sizeof(driver_block);
}

// Install driver block into specified offset.
bool Mp2kDriver::InstallDriver(uint8_t * rom, size_t rom_size, off_t offset, const std::map<std::string, VgmDriverParam>& params)
{
	char str[512];
	uint32_t sub_selectsong = params.at("sub_selectsong").getInteger();
	uint32_t sub_main = params.at("sub_main").getInteger();
	uint32_t sub_init = params.at("sub_init").getInteger();
	uint32_t sub_vsync = params.at("sub_vsync").getInteger();

	size_t driver_size = GetDriverSize(params);
	if (offset < 0 || (offset % 4) != 0 || (size_t) offset + driver_size > rom_size)
	{
		sprintf(str, "Could not install driver at 0x%08X (invalid offset)", offset);
		m_message = str;
		return false;
	}

	// install driver
	mput4l(sub_init | (sub_init != 0 ? 1 : 0), &driver_block[0xD8]);
	mput4l(sub_selectsong | (sub_selectsong != 0 ? 1 : 0), &driver_block[0xDC]);
	mput4l(sub_main | (sub_main != 0 ? 1 : 0), &driver_block[0xE0]);
	mput4l(sub_vsync | (sub_vsync != 0 ? 1 : 0), &driver_block[0xE4]);
	memcpy(&rom[offset], driver_block, driver_size);

	// modify entrypoint
	mput4l(0xEA000000 | (((offset - 8) / 4) & 0xFFFFFF), &rom[0]);

	return true;
}

// Return if the specified song is a duplicate of any previous songs.
bool Mp2kDriver::IsSongDuplicate(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params, uint32_t song)
{
	if (params.count("_array_songs") == 0)
	{
		return false;
	}
	uint32_t array_songs = params.at("_array_songs").getInteger();
	if (array_songs < 0x08000000 || array_songs >= 0x8000000 + rom_size)
	{
		return false;
	}
	array_songs = gba_address_to_offset(array_songs);
	return m4a_is_song_duplicate(rom, rom_size, array_songs, song);
}

uint32_t Mp2kDriver::find_m4a_selectsong(const uint8_t * rom, uint32_t rom_size)
{
	const uint8_t code_selectsong[0x1E] = {
		0x00, 0xB5, 0x00, 0x04, 0x07, 0x4A, 0x08, 0x49, 
		0x40, 0x0B, 0x40, 0x18, 0x83, 0x88, 0x59, 0x00, 
		0xC9, 0x18, 0x89, 0x00, 0x89, 0x18, 0x0A, 0x68, 
		0x01, 0x68, 0x10, 0x1C, 0x00, 0xF0,
	};
	const unsigned int code_maxdiff = 8;
	uint32_t offset;

	// check length
	if (rom_size < sizeof(code_selectsong))
	{
		return GSF_INVALID_OFFSET;
	}

	// search (function address must be 32bit-aligned)
	for (offset = 0; offset < rom_size - sizeof(code_selectsong); offset += 4)
	{
		// loose matching
		if (memcmploose(&rom[offset], code_selectsong, sizeof(code_selectsong), code_maxdiff) < code_maxdiff)
		{
			break;
		}
	}

	// return the offset if available
	if (offset + sizeof(code_selectsong) <= rom_size)
	{
		return offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

uint32_t Mp2kDriver::find_m4a_songtable(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_selectsong)
{
	uint32_t offset;

	if (offset_m4a_selectsong >= rom_size)
	{
		return GSF_INVALID_OFFSET;
	}
	if (offset_m4a_selectsong + 40 + 4 > rom_size)
	{
		return GSF_INVALID_OFFSET;
	}

	offset = mget4l(&rom[offset_m4a_selectsong + 40]);
	if (!is_gba_rom_address(offset))
	{
		return GSF_INVALID_OFFSET;
	}
	offset = gba_address_to_offset(offset);

	if (offset < rom_size)
	{
		return offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

uint32_t Mp2kDriver::find_m4a_main(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_selectsong)
{
	// PUSH    {LR}
	const uint8_t code_main[2] = {
		0x00, 0xB5,
	};
	const size_t code_searchrange = 0x20;
	uint32_t code_minoffset;
	uint32_t code_maxoffset;
	uint32_t offset;

	assert(code_searchrange % 4 == 0);

	// check length
	if (offset_m4a_selectsong >= rom_size || offset_m4a_selectsong < 4)
	{
		return GSF_INVALID_OFFSET;
	}
	if (rom_size < sizeof(code_main))
	{
		return GSF_INVALID_OFFSET;
	}

	// determine search range
	code_minoffset = (uint32_t) ((offset_m4a_selectsong >= code_searchrange) ?
		(offset_m4a_selectsong - code_searchrange) : 0);
	code_maxoffset = (uint32_t) ((offset_m4a_selectsong - 4 + sizeof(code_main) <= rom_size) ?
		(offset_m4a_selectsong - 4) : (rom_size - sizeof(code_main)));

	// backward search
	for (offset = code_maxoffset; offset >= code_minoffset; offset -= 4)
	{
		if (memcmp(&rom[offset], code_main, sizeof(code_main)) == 0)
		{
			break;
		}
	}

	// return the offset if available
	if (offset >= code_minoffset && offset <= code_maxoffset)
	{
		return offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

uint32_t Mp2kDriver::find_m4a_init(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_main)
{
	const uint8_t code_init[2][2] = { 
		{0x70, 0xB5},	// PUSH    {R4-R6,LR}
		{0xF0, 0xB5},	// PUSH    {R4-R7,LR}
	};
	const size_t code_patcount = sizeof(code_init[0]) / sizeof(code_init[0][0]);
	const size_t code_searchrange = 0x100;
	uint32_t code_minoffset;
	uint32_t code_maxoffset;
	uint32_t offset;

	assert(code_searchrange % 4 == 0);

	// check length
	if (offset_m4a_main >= rom_size || offset_m4a_main < 4)
	{
		return GSF_INVALID_OFFSET;
	}
	if (rom_size < sizeof(code_init[0]))
	{
		return GSF_INVALID_OFFSET;
	}

	// determine search range
	code_minoffset = (uint32_t) ((offset_m4a_main >= code_searchrange + 4) ?
		(offset_m4a_main - code_searchrange) : 4);
	code_maxoffset = (uint32_t) ((offset_m4a_main - 4 + sizeof(code_init[0]) <= rom_size) ?
		(offset_m4a_main - 4) : (rom_size - sizeof(code_init[0])));

	// backward search
	for (offset = code_maxoffset; offset >= code_minoffset; offset -= 4)
	{
		size_t code_patindex;
		for (code_patindex = 0; code_patindex < code_patcount; code_patindex++)
		{
			if (memcmp(&rom[offset], code_init[code_patindex], sizeof(code_init[0])) == 0)
			{
				break;
			}
		}
		if (code_patindex != code_patcount)
		{
			break;
		}
	}

	// return the offset if available
	if (offset >= code_minoffset && offset <= code_maxoffset)
	{
		return offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

uint32_t Mp2kDriver::find_m4a_vsync(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_init)
{
	// LDR     R0, =dword_3007FF0
	// LDR     R0, [R0]
	// LDR     R2, =0x68736D53
	// LDR     R3, [R0]
	// SUBS or CMP (early versions, Momotarou Matsuri for instance)
	BytePattern ptn_vsync(
		"\xa6\x48\x00\x68\xa6\x4a\x03\x68"
		,
		"?xxx?xxx"
		,
		0x08);
	// PUSH    {LR}
	// LDR     R0, =dword_3007FF0
	// LDR     R2, [R0]
	// LDR     R0, [R2]
	// LDR     R1, =0x978C92AD
	BytePattern ptn_vsync_fever(
		"\x00\xb5\x18\x48\x02\x68\x10\x68"
		"\x17\x49"
		,
		"xx?xxxxx"
		"?x"
		,
		0x0a);

	const size_t code_searchrange = 0x1800; // 0x1000 might be good, but longer is safer anyway :)
	uint32_t code_minoffset;
	uint32_t code_maxoffset;
	uint32_t offset;

	assert(code_searchrange % 4 == 0);

	// check length
	if (rom_size < 4 || offset_m4a_init >= (rom_size - 4) || offset_m4a_init < 4)
	{
		return GSF_INVALID_OFFSET;
	}
	if (rom_size < ptn_vsync.length())
	{
		return GSF_INVALID_OFFSET;
	}

	// determine search range
	code_minoffset = (uint32_t) ((offset_m4a_init >= code_searchrange + 4) ?
		(offset_m4a_init - code_searchrange) : 4);
	code_maxoffset = (uint32_t) ((offset_m4a_init - 4 + ptn_vsync.length() <= rom_size) ?
		(offset_m4a_init - 4) : (rom_size - ptn_vsync.length()));

	// backward search
	for (offset = code_maxoffset; offset >= code_minoffset; offset -= 4)
	{
		if (ptn_vsync.match(&rom[offset], rom_size - offset))
		{
			// Momotarou Matsuri:
			// check "BX LR" and avoid false-positive
			if (offset + 0x0e <= rom_size && mget2l(&rom[offset + 0x0c]) == 0x4770)
			{
				continue;
			}

			break;
		}
	}

	// return the offset if available
	if (offset >= code_minoffset && offset <= code_maxoffset)
	{
		return offset;
	}

	// else... prepare for extra search (Puyo Pop Fever, Precure, etc.)
	code_minoffset = (uint32_t) (offset_m4a_init + 4);
	code_maxoffset = (uint32_t) (offset_m4a_init + code_searchrange);
	if (code_maxoffset > rom_size)
	{
		code_maxoffset = rom_size;
	}

	// forward search
	for (offset = code_minoffset; offset < code_maxoffset; offset += 4)
	{
		if (ptn_vsync_fever.match(&rom[offset], rom_size - offset))
		{
			break;
		}
	}

	// return the offset if available
	if (offset >= code_minoffset && offset < code_maxoffset)
	{
		return offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

unsigned int Mp2kDriver::m4a_get_song_count(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_songtable)
{
	unsigned int song_count = 0;
	uint32_t offset;

	// check length
	if (offset_m4a_songtable == GSF_INVALID_OFFSET ||
		rom_size < 8 || offset_m4a_songtable > rom_size - 8)
	{
		return 0;
	}

	// parse song table
	offset = offset_m4a_songtable;
	while (offset + 8 <= rom_size)
	{
		uint32_t addr = mget4l(&rom[offset]);
		if (!is_gba_rom_address(addr))
		{
			break;
		}

		song_count++;
		offset += 8;
	}
	return song_count;
}

bool Mp2kDriver::m4a_is_song_duplicate(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_songtable, unsigned int song_index)
{
	uint32_t offset;
	uint32_t src_offset;

	// check length
	if (offset_m4a_songtable == GSF_INVALID_OFFSET ||
		rom_size < 8 || offset_m4a_songtable > rom_size - 8)
	{
		return false;
	}
	src_offset = offset_m4a_songtable + (song_index * 8);
	if (src_offset + 8 > rom_size)
	{
		return false;
	}

	offset = offset_m4a_songtable;
	while (offset + 8 <= rom_size && offset < src_offset)
	{
		if (memcmp(&rom[offset], &rom[src_offset], 8) == 0)
		{
			return true;
		}
		offset += 8;
	}
	return false;
}
