/*
** Simple utility to convert a GBA Rom to a GSF.
** Based on EXE2PSF code, written by Neill Corlett
** Released under the terms of the GNU General Public License
**
** You need zlib to compile this.
** It's available at http://www.gzip.org/zlib/
*/

#define APP_NAME	"Saptapper"
#define APP_VER		"[2014-01-15]"
#define APP_DESC	"Automated GSF ripper tool"
#define APP_AUTHOR	"Caitsith2, revised by loveemu <http://github.com/loveemu/saptapper>"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <map>

#include "zlib.h"

#include "saptapper.h"
#include "BytePattern.h"
#include "cbyteio.h"
#include "cpath.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define _chdir(s)	chdir((s))
#define _mkdir(s)	mkdir((s), 0777)
#define _rmdir(s)	rmdir((s))
#endif

unsigned int memcmploose(const void *buf1, const void *buf2, size_t n, unsigned int maxdiff)
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

void Saptapper::put_gsf_exe_header(uint8_t *exe, uint32_t entrypoint, uint32_t load_offset, uint32_t rom_size)
{
	mput4l(entrypoint, &exe[0]);
	mput4l(load_offset, &exe[4]);
	mput4l(rom_size, &exe[8]);
}

bool Saptapper::exe2gsf(const std::string& gsf_path, uint8_t *exe, size_t exe_size)
{
	std::map<std::string, std::string> tags;
	return exe2gsf(gsf_path, exe, exe_size, tags);
}

#define CHUNK 16384
bool Saptapper::exe2gsf(const std::string& gsf_path, uint8_t *exe, size_t exe_size, std::map<std::string, std::string>& tags)
{
	FILE *gsf_file = NULL;

	z_stream z;
	uint8_t zbuf[CHUNK];
	uLong zcrc;
	uLong zlen;
	int zflush;
	int zret;

	// check exe size
	if (exe_size > MAX_GSF_EXE_SIZE)
	{
		return false;
	}

	// open output file
	gsf_file = fopen(gsf_path.c_str(), "wb");
	if (gsf_file == NULL)
	{
		return false;
	}

	// write PSF header
	// (EXE length and CRC will be set later)
	fwrite(PSF_SIGNATURE, strlen(PSF_SIGNATURE), 1, gsf_file);
	fputc(GSF_PSF_VERSION, gsf_file);
	fput4l(0, gsf_file);
	fput4l(0, gsf_file);
	fput4l(0, gsf_file);

	// init compression
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK)
	{
		return false;
	}

	// compress exe
	z.next_in = exe;
	z.avail_in = exe_size;
	z.next_out = zbuf;
	z.avail_out = CHUNK;
	zflush = Z_FINISH;
	zcrc = crc32(0L, Z_NULL, 0);
	do
	{
		// compress
		zret = deflate(&z, zflush);
		if (zret != Z_STREAM_END && zret != Z_OK)
		{
			deflateEnd(&z);
			fclose(gsf_file);
			return false;
		}

		// write compressed data
		zlen = CHUNK - z.avail_out;
		if (zlen != 0)
		{
			if (fwrite(zbuf, zlen, 1, gsf_file) != 1)
			{
				deflateEnd(&z);
				fclose(gsf_file);
				return false;
			}
			zcrc = crc32(zcrc, zbuf, zlen);
		}

		// give space for next chunk
		z.next_out = zbuf;
		z.avail_out = CHUNK;
	} while (zret != Z_STREAM_END);

	// set EXE info to PSF header
	fseek(gsf_file, 8, SEEK_SET);
	fput4l(z.total_out, gsf_file);
	fput4l(zcrc, gsf_file);
	fseek(gsf_file, 0, SEEK_END);

	// end compression
	deflateEnd(&z);

	// write tags
	if (!tags.empty())
	{
		fwrite(PSF_TAG_SIGNATURE, strlen(PSF_TAG_SIGNATURE), 1, gsf_file);

		for (std::map<std::string, std::string>::iterator it = tags.begin(); it != tags.end(); ++it)
		{
			const std::string& key = it->first;
			const std::string& value = it->second;
			std::istringstream value_reader(value);
			std::string line;

			// process for each lines
			while (std::getline(value_reader, line))
			{
				if (fprintf(gsf_file, "%s=%s\n", key.c_str(), line.c_str()) < 0)
				{
					fclose(gsf_file);
					return false;
				}
			}
		}
	}

	fclose(gsf_file);
	return true;
}

bool Saptapper::make_minigsf(const std::string& gsf_path, uint32_t address, size_t size, uint32_t num, std::map<std::string, std::string>& tags)
{
	uint8_t exe[GSF_EXE_HEADER_SIZE + 4];

	// limit size
	if (size > 4)
	{
		return false;
	}

	// make exe
	put_gsf_exe_header(exe, GBA_ENTRYPOINT, address, size);
	mput4l(num, &exe[GSF_EXE_HEADER_SIZE]);

	// write minigsf file
	return exe2gsf(gsf_path, exe, GSF_EXE_HEADER_SIZE + size, tags);
}

const char* Saptapper::get_gsflib_error(EGsfLibResult gsflibstat)
{
	switch (gsflibstat)
	{
	case GSFLIB_OK:
		return "Operation finished successfully";

	case GSFLIB_NOMAIN:
		return "sappy_main not found";

	case GSFLIB_NOSELECT:
		return "sappy_selectsongbynum not found";

	case GSFLIB_NOINIT:
		return "sappy_init not found";

	case GSFLIB_NOVSYNC:
		return "sappy_vsync not found";

	case GSFLIB_NOSPACE:
		return "Insufficient space found";

	case GSFLIB_ZLIB_ERR:
		return "GSFLIB zlib compression error";

	case GSFLIB_INFILE_E:
		return "File read error";

	case GSFLIB_OTFILE_E:
		return "File write error";

	default:
		return "Undefined error";
	}
}

bool Saptapper::load_rom(uint8_t* rom, size_t rom_size)
{
	close_rom();

	// check length
	if (rom_size > MAX_GBA_ROM_SIZE)
	{
		return false;
	}

	// allocate memory
	this->rom_exe = new uint8_t[GSF_EXE_HEADER_SIZE + rom_size];
	this->rom = &rom_exe[GSF_EXE_HEADER_SIZE];
	this->rom_size = rom_size;

	// read ROM
	memcpy(this->rom, rom, rom_size);

	return true;
}

bool Saptapper::load_rom_file(const std::string& rom_path)
{
	close_rom();

	// check length
	off_t rom_size_off = path_getfilesize(rom_path.c_str());
	if (rom_size_off < 0 || rom_size_off > MAX_GBA_ROM_SIZE)
	{
		return false;
	}
	size_t rom_size = (size_t) rom_size_off;

	// open the file
	FILE *rom_file = fopen(rom_path.c_str(), "rb");
	if (rom_file == NULL)
	{
		return false;
	}

	// allocate memory
	this->rom_exe = new uint8_t[GSF_EXE_HEADER_SIZE + rom_size];
	this->rom = &rom_exe[GSF_EXE_HEADER_SIZE];
	this->rom_size = rom_size;

	// read ROM
	if (fread(rom, 1, rom_size, rom_file) != rom_size)
	{
		fclose(rom_file);
		close_rom();
		return false;
	}

	fclose(rom_file);
	return true;
}

void Saptapper::close_rom(void)
{
	if (rom_exe != NULL)
	{
		uninstall_driver();

		delete [] rom_exe;
		rom_exe = NULL;
		rom = NULL;
	}
	rom_size = 0;
}

bool Saptapper::install_driver(uint8_t* driver_block, uint32_t offset, size_t size)
{
	uninstall_driver();

	// check range, alignments, etc.
	if (driver_block == NULL || (offset % 4) != 0 || size == 0 ||
		(offset + size) > rom_size || rom == NULL || rom_size < 4)
	{
		return false;
	}

	// allocate memory for backup
	rom_patch_backup = new uint8_t[size];
	if (rom_patch_backup == NULL)
	{
		return false;
	}

	// make a backup of unpatched ROM
	rom_patch_entrypoint_backup = mget4l(&rom[0]);
	memcpy(rom_patch_backup, &rom[offset], size);
	rom_patch_offset = offset;
	rom_patch_size = size;

	// patch ROM
	mput4l(0xEA000000 | (((offset - 8) / 4) & 0xFFFFFF), &rom[0]);
	memcpy(&rom[offset], driver_block, size);

	return true;
}

void Saptapper::uninstall_driver(void)
{
	if (rom_patch_backup != NULL)
	{
		memcpy(&rom[rom_patch_offset], rom_patch_backup, rom_patch_size);
		mput4l(rom_patch_entrypoint_backup, &rom[0]);

		delete [] rom_patch_backup;
		rom_patch_backup = NULL;
	}
	rom_patch_offset = GSF_INVALID_OFFSET;
	rom_patch_size = 0;
}

uint32_t Saptapper::find_m4a_selectsong(void)
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

uint32_t Saptapper::find_m4a_songtable(uint32_t offset_m4a_selectsong)
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

uint32_t Saptapper::find_m4a_main(uint32_t offset_m4a_selectsong)
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
	code_minoffset = (offset_m4a_selectsong >= code_searchrange) ?
		(offset_m4a_selectsong - code_searchrange) : 0;
	code_maxoffset = (offset_m4a_selectsong - 4 + sizeof(code_main) <= rom_size) ?
		(offset_m4a_selectsong - 4) : (rom_size - sizeof(code_main));

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

uint32_t Saptapper::find_m4a_init(uint32_t offset_m4a_main)
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
	code_minoffset = (offset_m4a_main >= code_searchrange) ?
		(offset_m4a_main - code_searchrange) : 0;
	code_maxoffset = (offset_m4a_main - 4 + sizeof(code_init[0]) <= rom_size) ?
		(offset_m4a_main - 4) : (rom_size - sizeof(code_init[0]));

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

uint32_t Saptapper::find_m4a_vsync(uint32_t offset_m4a_init)
{
	// Note that we do not use the first 5 bytes
	// LDR     R0, [PC, #0x298]
	// LDR     R0, [R0]
	// LDR     R2, [PC, #0x298]
	// LDR     R3, [R0]
	// SUBS    R3, R3, R2
	const uint8_t code_vsync[10] = {
		0xA6, 0x48, 0x00, 0x68, 0xA6, 0x4A, 0x03, 0x68, 0x9B, 0x1A,
	};

	const size_t code_searchrange = 0x800;
	const uint32_t code_cmpoffset = 5;
	uint32_t code_minoffset;
	uint32_t code_maxoffset;
	uint32_t offset;

	assert(code_searchrange % 4 == 0);
	assert(code_cmpoffset < sizeof(code_vsync));

	// check length
	if (offset_m4a_init >= rom_size || offset_m4a_init < 4)
	{
		return GSF_INVALID_OFFSET;
	}
	if (rom_size < sizeof(code_vsync))
	{
		return GSF_INVALID_OFFSET;
	}

	// determine search range
	code_minoffset = (offset_m4a_init >= code_searchrange) ?
		(offset_m4a_init - code_searchrange) : 0;
	code_maxoffset = (offset_m4a_init - 4 + sizeof(code_vsync) <= rom_size) ?
		(offset_m4a_init - 4) : (rom_size - sizeof(code_vsync));

	// backward search
	for (offset = code_maxoffset; offset >= code_minoffset; offset -= 4)
	{
		if (memcmp(&rom[offset + code_cmpoffset], &code_vsync[code_cmpoffset], sizeof(code_vsync) - code_cmpoffset) == 0)
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

Saptapper::EGsfLibResult Saptapper::find_m4a_addresses(void)
{
	if (manual_offset_m4a_selectsong == GSF_INVALID_OFFSET)
	{
		offset_m4a_selectsong = find_m4a_selectsong();
	}
	else
	{
		if (manual_offset_m4a_selectsong < rom_size)
		{
			offset_m4a_selectsong = manual_offset_m4a_selectsong;
		}
		else
		{
			offset_m4a_selectsong = GSF_INVALID_OFFSET;
		}
	}

	if (manual_offset_m4a_songtable == GSF_INVALID_OFFSET)
	{
		offset_m4a_songtable = find_m4a_songtable(offset_m4a_selectsong);
	}
	else
	{
		if (manual_offset_m4a_songtable < rom_size)
		{
			offset_m4a_songtable = manual_offset_m4a_songtable;
		}
		else
		{
			offset_m4a_songtable = GSF_INVALID_OFFSET;
		}
	}

	if (manual_offset_m4a_main == GSF_INVALID_OFFSET)
	{
		offset_m4a_main = find_m4a_main(offset_m4a_selectsong);
	}
	else
	{
		if (manual_offset_m4a_main < rom_size)
		{
			offset_m4a_main = manual_offset_m4a_main;
		}
		else
		{
			offset_m4a_main = GSF_INVALID_OFFSET;
		}
	}

	if (manual_offset_m4a_init == GSF_INVALID_OFFSET)
	{
		offset_m4a_init = find_m4a_init(offset_m4a_main);
	}
	else
	{
		if (manual_offset_m4a_init < rom_size)
		{
			offset_m4a_init = manual_offset_m4a_init;
		}
		else
		{
			offset_m4a_init = GSF_INVALID_OFFSET;
		}
	}

	if (manual_offset_m4a_vsync == GSF_INVALID_OFFSET)
	{
		offset_m4a_vsync = find_m4a_vsync(offset_m4a_init);
	}
	else
	{
		if (manual_offset_m4a_vsync < rom_size)
		{
			offset_m4a_vsync = manual_offset_m4a_vsync;
		}
		else
		{
			offset_m4a_vsync = GSF_INVALID_OFFSET;
		}
	}

	if (offset_m4a_selectsong == GSF_INVALID_OFFSET)
	{
		return GSFLIB_NOSELECT;
	}
	else if (offset_m4a_main == GSF_INVALID_OFFSET)
	{
		return GSFLIB_NOMAIN;
	}
	else if (offset_m4a_init == GSF_INVALID_OFFSET)
	{
		return GSFLIB_NOINIT;
	}
	else if (offset_m4a_vsync == GSF_INVALID_OFFSET)
	{
		return GSFLIB_NOVSYNC;
	}
	//else if (offset_m4a_songtable == GSF_INVALID_OFFSET)
	//{
	//	return GSFLIB_NOSONGTABLE;
	//}
	return GSFLIB_OK;
}

uint32_t Saptapper::find_free_space(size_t size, uint8_t filler)
{
	uint32_t offset;
	uint32_t space_offset;
	size_t space_size = 0;
	uint32_t max_space_offset;
	size_t max_space_size = 0;

	// check length
	if (rom_size > 0 && size > rom_size)
	{
		return GSF_INVALID_OFFSET;
	}

	// search (ARM instruction must be 32bit-aligned)
	offset = 0;
	while (offset < rom_size - size)
	{
		if (rom[offset] == filler)
		{
			if (space_size == 0)
			{
				space_offset = offset;
			}
			space_size++;
			offset++;
		}
		else
		{
			if (space_size > max_space_size)
			{
				max_space_offset = space_offset;
				max_space_size = space_size;
				if (max_space_size >= size && !prefer_larger_free_space)
				{
					break;
				}
			}
			space_size = 0;
			offset += 4 - (offset % 4);
		}
	}

	// return the offset if available
	if (max_space_size >= size)
	{
		return max_space_offset;
	}
	else
	{
		return GSF_INVALID_OFFSET;
	}
}

uint32_t Saptapper::find_free_space(size_t size)
{
	uint32_t offset;

	offset = find_free_space(size, 0xFF);
	if (offset == GSF_INVALID_OFFSET)
	{
		offset = find_free_space(size, 0x00);
	}
	return offset;
}

unsigned int Saptapper::get_song_count(uint32_t offset_m4a_songtable)
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

bool Saptapper::is_song_duplicate(uint32_t offset_m4a_songtable, unsigned int song_index)
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

Saptapper::EGsfLibResult Saptapper::make_gsflib(const std::string& gsf_path)
{
	uint8_t sappyblock[284] =
	{
		0x00, 0x80, 0x2D, 0xE9, 0x01, 0x00, 0xBD, 0xE8, 0x60, 0x10, 0xA0, 0xE3, 0x00, 0x20, 0x90, 0xE5, 
		0x04, 0x00, 0x80, 0xE2, 0x04, 0x10, 0x41, 0xE2, 0x00, 0x00, 0x51, 0xE3, 0xFA, 0xFF, 0xFF, 0x1A, 
		0x0F, 0x00, 0x00, 0xEA, 0x53, 0x61, 0x70, 0x70, 0x79, 0x20, 0x44, 0x72, 0x69, 0x76, 0x65, 0x72, 
		0x20, 0x52, 0x69, 0x70, 0x70, 0x65, 0x72, 0x20, 0x62, 0x79, 0x20, 0x43, 0x61, 0x69, 0x74, 0x53, 
		0x69, 0x74, 0x68, 0x32, 0x5C, 0x5A, 0x6F, 0x6F, 0x70, 0x64, 0x2C, 0x20, 0x28, 0x63, 0x29, 0x20, 
		0x32, 0x30, 0x30, 0x34, 0x2C, 0x20, 0x6C, 0x6F, 0x76, 0x65, 0x65, 0x6D, 0x75, 0x20, 0x32, 0x30, 
		0x31, 0x34, 0x2E, 0x00, 0x00, 0x40, 0x2D, 0xE9, 0x80, 0x00, 0x9F, 0xE5, 0x21, 0x00, 0x00, 0xEB, 
		0x9C, 0x00, 0x9F, 0xE5, 0x00, 0x80, 0x2D, 0xE9, 0x02, 0x00, 0xBD, 0xE8, 0x30, 0x10, 0x81, 0xE2, 
		0x00, 0x10, 0x80, 0xE5, 0x01, 0x03, 0xA0, 0xE3, 0x08, 0x10, 0xA0, 0xE3, 0x04, 0x10, 0x80, 0xE5, 
		0x01, 0x10, 0xA0, 0xE3, 0x00, 0x12, 0x80, 0xE5, 0x08, 0x12, 0x80, 0xE5, 0x74, 0x00, 0x9F, 0xE5, 
		0x40, 0x10, 0x9F, 0xE5, 0x16, 0x00, 0x00, 0xEB, 0x00, 0x00, 0x02, 0xEF, 0xFD, 0xFF, 0xFF, 0xEA, 
		0x00, 0x40, 0x2D, 0xE9, 0x38, 0x00, 0x9F, 0xE5, 0x0E, 0x00, 0x00, 0xEB, 0x28, 0x00, 0x9F, 0xE5, 
		0x0C, 0x00, 0x00, 0xEB, 0x01, 0x03, 0xA0, 0xE3, 0x01, 0x18, 0xA0, 0xE3, 0x01, 0x10, 0x81, 0xE2, 
		0x00, 0x12, 0x80, 0xE5, 0x38, 0x00, 0x9F, 0xE5, 0x04, 0x10, 0x00, 0xE5, 0x01, 0x00, 0xBD, 0xE8, 
		0x10, 0xFF, 0x2F, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xE3, 0x03, 0x00, 0x00, 0x0A, 
		0x10, 0xFF, 0x2F, 0xE1, 0x00, 0x00, 0x51, 0xE3, 0x00, 0x00, 0x00, 0x0A, 0x11, 0xFF, 0x2F, 0xE1, 
		0x1E, 0xFF, 0x2F, 0xE1, 0xFC, 0x7F, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00
	};
	uint8_t *gsf_driver_block = sappyblock;
	size_t gsf_driver_size = sizeof(sappyblock);

	EGsfLibResult gsflibstat;

	// obtain necessary addresses
	gsflibstat = find_m4a_addresses();
	if (gsflibstat != GSFLIB_OK)
	{
		return gsflibstat;
	}

	// set addresses to the driver block
	mput4l(gba_offset_to_address(offset_m4a_selectsong | 1), &sappyblock[0xE8]);
	mput4l(gba_offset_to_address(offset_m4a_main | 1), &sappyblock[0xEC]);
	mput4l(gba_offset_to_address(offset_m4a_init | 1), &sappyblock[0xF0]);
	mput4l(gba_offset_to_address(offset_m4a_vsync | 1), &sappyblock[0xF4]);

	// determine gsf driver offset
	if (manual_offset_gsf_driver == GSF_INVALID_OFFSET)
	{
		offset_gsf_driver = find_free_space(0x200) + (0x200 - gsf_driver_size);
	}
	else
	{
		if (manual_offset_gsf_driver < rom_size)
		{
			offset_gsf_driver = manual_offset_gsf_driver;
		}
		else
		{
			offset_gsf_driver = GSF_INVALID_OFFSET;
		}
	}
	// check gsf driver offset
	if (offset_gsf_driver == GSF_INVALID_OFFSET)
	{
		return GSFLIB_NOSPACE;
	}

	// determine minigsf offset
	offset_minigsf_number = offset_gsf_driver + 0x118;

	// install driver temporarily
	install_driver(gsf_driver_block, offset_gsf_driver, gsf_driver_size);

	// create gsflib file
	put_gsf_exe_header(rom_exe, GBA_ENTRYPOINT, GBA_ENTRYPOINT, rom_size);
	if (!exe2gsf(gsf_path, rom_exe, GSF_EXE_HEADER_SIZE + rom_size))
	{
		gsflibstat = GSFLIB_OTFILE_E;
	}

	// uninstall driver block
	uninstall_driver();

	return gsflibstat;
}

bool Saptapper::make_gsf_set(const std::string& rom_path)
{
	bool result = false;
	EGsfLibResult gsflibstat = GSFLIB_OK;
	std::map<std::string, std::string> tags;

	// get separator positions
	const char *c_rom_path = rom_path.c_str();
	const char *c_rom_base = path_findbase(c_rom_path);
	const char *c_rom_ext = path_findext(c_rom_path);
	// extract directory path and filename
	std::string rom_dir = rom_path.substr(0, c_rom_base - c_rom_path);
	std::string rom_basename = rom_path.substr(c_rom_base - c_rom_path,
		(c_rom_ext != NULL) ? (c_rom_ext - c_rom_base) : std::string::npos);
	// construct some new paths
	std::string gsf_dir = rom_dir + rom_basename;
	std::string gsflib_name = rom_basename + ".gsflib";
	std::string gsflib_path = gsf_dir + PATH_SEPARATOR_STR + gsflib_name;

	if (!quiet)
	{
		printf("%s\n", c_rom_base);
		printf("----------------------------------------\n");
		printf("\n");
	}

	// load ROM image
	if (!load_rom_file(rom_path))
	{
		fprintf(stderr, "Error: %s - Could not be loaded\n", rom_path.c_str());
		return false;
	}

	std::string rom_title = std::string((const char*)&rom[0xA0], 12);
	std::string rom_id = std::string((const char*)&rom[0xAC], 4);
	if (!quiet)
	{
		printf("%s (%s)\n", rom_title.c_str(), rom_id.c_str());
		printf("\n");
	}

	// create output directory
	_mkdir(gsf_dir.c_str());
	if (!path_isdir(gsf_dir.c_str()))
	{
		fprintf(stderr, "Error: %s - Directory could not be created\n", gsf_dir.c_str());
		close_rom();
		return false;
	}

	// create gsflib
	gsflibstat = make_gsflib(gsflib_path);

	// determine minigsf constants
	uint32_t minigsfoffset = offset_minigsf_number;
	unsigned int minigsfcount = (offset_m4a_songtable != GSF_INVALID_OFFSET) ? get_song_count(offset_m4a_songtable) : 0;
	unsigned int minigsferrors = 0;
	unsigned int minigsfdupes = 0;

	// show address info
	if (!quiet)
	{
		if (offset_m4a_init != GSF_INVALID_OFFSET)
		{
			printf("- sappy_init = 0x%08X\n", gba_offset_to_address(offset_m4a_init));
		}
		else
		{
			printf("- sappy_init = undefined\n");
		}

		if (offset_m4a_main != GSF_INVALID_OFFSET)
		{
			printf("- sappy_main = 0x%08X\n", gba_offset_to_address(offset_m4a_main));
		}
		else
		{
			printf("- sappy_main = undefined\n");
		}

		if (offset_m4a_selectsong != GSF_INVALID_OFFSET)
		{
			printf("- sappy_selectsongbynum = 0x%08X\n", gba_offset_to_address(offset_m4a_selectsong));
		}
		else
		{
			printf("- sappy_selectsongbynum = undefined\n");
		}

		if (offset_m4a_vsync != GSF_INVALID_OFFSET)
		{
			printf("- sappy_vsync = 0x%08X\n", gba_offset_to_address(offset_m4a_vsync));
		}
		else
		{
			printf("- sappy_vsync = undefined\n");
		}

		if (offset_m4a_songtable != GSF_INVALID_OFFSET)
		{
			printf("- sappy_songs = 0x%08X\n", gba_offset_to_address(offset_m4a_songtable));
		}
		else
		{
			printf("- sappy_songs = undefined\n");
		}

		if (offset_gsf_driver != GSF_INVALID_OFFSET)
		{
			printf("- gsf_driver_block = 0x%08X\n", gba_offset_to_address(offset_gsf_driver));
		}
		else
		{
			printf("- gsf_driver_block = undefined\n");
		}

		printf("\n");
	}

	// gsflib succeeded?
	if (gsflibstat != GSFLIB_OK)
	{
		fprintf(stderr, "Error: %s - %s\n", rom_path.c_str(), get_gsflib_error(gsflibstat));
		_rmdir(gsf_dir.c_str());
		close_rom();
		return false;
	}

	// determine minigsf size
	size_t minigsfsize = 0;
	do
	{
		minigsfsize++;
	} while((minigsfcount >> (minigsfsize * 8)) != 0);

	// set minigsf tags
	tags["_lib"] = gsflib_name;
	if (tag_gsfby.empty())
	{
		tags["gsfby"] = "Saptapper";
	}
	else
	{
		if (tag_gsfby == "Caitsith2")
		{
			tags["gsfby"] = tag_gsfby;
		}
		else
		{
			tags["gsfby"] = "Saptapper, with help of " + tag_gsfby;
		}
	}

	result = true;
	for (unsigned int minigsfindex = 0; minigsfindex < minigsfcount; minigsfindex++)
	{
		char minigsfname[PATH_MAX];

		sprintf(minigsfname, "%s.%04d.minigsf", rom_basename.c_str(), minigsfindex);
		std::string minigsf_path = gsf_dir + PATH_SEPARATOR_STR + minigsfname;

		if (!is_song_duplicate(offset_m4a_songtable, minigsfindex))
		{
			if (!make_minigsf(minigsf_path, minigsfoffset, minigsfsize, minigsfindex, tags))
			{
				minigsferrors++;
			}
		}
		else
		{
			minigsfdupes++;
		}
	}

	// show song count
	if (quiet)
	{
		if (minigsfcount == 0)
		{
			fprintf(stderr, "No songs\n");
		}
		if (minigsferrors > 0)
		{
			fprintf(stderr, "%d error(s)\n", minigsferrors);
		}
	}
	else
	{
		printf("%d succeeded", minigsfcount - minigsfdupes - minigsferrors);
		if (minigsferrors > 0)
		{
			printf(", %d failed", minigsferrors);
		}
		if (minigsfdupes > 0)
		{
			printf(", %d skipped", minigsfdupes);
		}
		printf("\n");
		printf("\n");
	}

	close_rom();
	return result;
}

void printUsage(const char *cmd)
{
	const char *availableOptions[] = {
		"--help", "Show this help",
		"-v, --verbose", "Output ripping info to STDOUT",
		"--offset-selectsong [0xXXXXXXXX]", "Specify the offset of sappy_selectsong function",
		"--offset-songtable [0xXXXXXXXX]", "Specify the offset of song table (well known Sappy offset)",
		"--offset-main [0xXXXXXXXX]", "Specify the offset of sappy_main function",
		"--offset-init [0xXXXXXXXX]", "Specify the offset of sappy_init function",
		"--offset-vsync [0xXXXXXXXX]", "Specify the offset of sappy_vsync function",
		"--tag-gsfby [name]", "Specify the nickname of GSF ripper",
		"--find-freespace [ROM.gba] [size]", "Find free space and quit",
	};

	printf("%s %s\n", APP_NAME, APP_VER);
	printf("======================\n");
	printf("\n");
	printf("%s. Created by %s.\n", APP_DESC, APP_AUTHOR);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: %s <GBA Files>\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");

	for (int i = 0; i < sizeof(availableOptions) / sizeof(availableOptions[0]); i += 2)
	{
		printf("%s\n", availableOptions[i]);
		printf("  : %s\n", availableOptions[i + 1]);
		printf("\n");
	}
}

int main(int argc, char **argv)
{
	Saptapper app;
	bool result;
	int argnum;
	int argi;

	char *strtol_endp;
	unsigned long ul;

	app.set_quiet(true);

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0)
		{
			app.set_quiet(false);
		}
		else if (strcmp(argv[argi], "--offset-selectsong") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 16);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			// GBA ROM address to offset
			if (ul >= 0x08000000 && ul <= 0x09FFFFFF)
			{
				ul &= 0x01FFFFFF;
			}
			app.set_m4a_selectsong(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "--offset-songtable") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 16);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			// GBA ROM address to offset
			if (ul >= 0x08000000 && ul <= 0x09FFFFFF)
			{
				ul &= 0x01FFFFFF;
			}
			app.set_m4a_songtable(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "--offset-main") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 16);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			// GBA ROM address to offset
			if (ul >= 0x08000000 && ul <= 0x09FFFFFF)
			{
				ul &= 0x01FFFFFF;
			}
			app.set_m4a_main(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "--offset-init") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 16);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			// GBA ROM address to offset
			if (ul >= 0x08000000 && ul <= 0x09FFFFFF)
			{
				ul &= 0x01FFFFFF;
			}
			app.set_m4a_init(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "--offset-vsync") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 16);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			// GBA ROM address to offset
			if (ul >= 0x08000000 && ul <= 0x09FFFFFF)
			{
				ul &= 0x01FFFFFF;
			}
			app.set_m4a_vsync(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "--tag-gsfby") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			app.set_tag_gsfby(argv[argi + 1]);
			argi++;
		}
		else if (strcmp(argv[argi], "--find-freespace") == 0)
		{
			if (argi + 3 != argc)
			{
				fprintf(stderr, "Error: Too few/more arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 2], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			// load ROM file
			std::string rom_path(argv[argi + 1]);
			if (!app.load_rom_file(rom_path))
			{
				fprintf(stderr, "Error: %s - Could not be loaded\n", rom_path.c_str());
				return EXIT_FAILURE;
			}

			// find free space
			uint32_t offset = app.find_free_space(ul);
			if (offset == GSF_INVALID_OFFSET)
			{
				fprintf(stderr, "Error: Insufficient space found\n");
				return EXIT_FAILURE;
			}
			printf("0x%08X\n", Saptapper::gba_offset_to_address(offset));

			argi++;
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "--largespace") == 0)
		{
			// experimental: it's secret to everybody :)
			app.set_prefer_larger_free_space(true);
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	argnum = argc - argi;
	if (argnum == 0)
	{
		fprintf(stderr, "Error: No input files.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "Run \"%s --help\" for help.\n", argv[0]);
		return EXIT_FAILURE;
	}

	result = true;
	for (; argi < argc; argi++)
	{
		result = result && app.make_gsf_set(argv[argi]);
	}
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
