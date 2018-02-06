/*
** Simple utility to convert a GBA Rom to a GSF.
** Based on EXE2PSF code, written by Neill Corlett
** Released under the terms of the GNU General Public License
**
** You need zlib to compile this.
** It's available at http://www.gzip.org/zlib/
*/

#define APP_NAME	"Saptapper"
#define APP_VER		"[2015-04-07]"
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

#include <zlib.h>

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
	if (deflateInit(&z, Z_BEST_COMPRESSION) != Z_OK)
	{
		return false;
	}

	// compress exe
	z.next_in = exe;
	z.avail_in = (uInt) exe_size;
	z.next_out = zbuf;
	z.avail_out = CHUNK;
	zcrc = crc32(0L, Z_NULL, 0);
	do
	{
		if (z.avail_in == 0)
		{
			zflush = Z_FINISH;
		}
		else
		{
			zflush = Z_NO_FLUSH;
		}

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
	uint8_t exe[GSF_EXE_HEADER_SIZE + 256];
	memset(exe, 0, GSF_EXE_HEADER_SIZE + 256);

	// limit size
	if (size > 256)
	{
		return false;
	}

	// make exe
	put_gsf_exe_header(exe, GBA_ENTRYPOINT, address, (uint32_t) size);
	mput4l(num, &exe[GSF_EXE_HEADER_SIZE]);

	// write minigsf file
	return exe2gsf(gsf_path, exe, GSF_EXE_HEADER_SIZE + size, tags);
}

bool Saptapper::set_gsf_driver(uint8_t* driver_block, size_t size, uint32_t minigsf_offset, bool thumb, bool use_main)
{
	unset_gsf_driver();

	if (!use_main && thumb)
	{
		return false;
	}

	if (driver_block == NULL || size == 0 || size > MAX_GBA_ROM_SIZE || minigsf_offset >= size)
	{
		return false;
	}

	manual_gsf_driver = new uint8_t[size];
	if (manual_gsf_driver == NULL)
	{
		return false;
	}
	memcpy(manual_gsf_driver, driver_block, size);
	manual_gsf_driver_size = size;
	manual_minigsf_offset = minigsf_offset;
	manual_driver_is_thumb = thumb;
	manual_driver_is_main = use_main;

	return true;
}

bool Saptapper::set_gsf_driver_file(const std::string& driver_path, uint32_t minigsf_offset, bool thumb, bool use_main)
{
	unset_gsf_driver();

	if (!use_main && thumb)
	{
		return false;
	}

	// check length
	off_t driver_size_off = path_getfilesize(driver_path.c_str());
	if (driver_size_off < 0 || driver_size_off > MAX_GBA_ROM_SIZE || minigsf_offset >= (size_t)driver_size_off)
	{
		return false;
	}
	size_t driver_size = (size_t) driver_size_off;

	// open the file
	FILE *driver_file = fopen(driver_path.c_str(), "rb");
	if (driver_file == NULL)
	{
		return false;
	}

	// set the driver info
	manual_gsf_driver = new uint8_t[driver_size];
	manual_gsf_driver_size = driver_size;
	manual_minigsf_offset = minigsf_offset;
	manual_driver_is_thumb = thumb;
	manual_driver_is_main = use_main;

	// read the file
	if (fread(manual_gsf_driver, 1, driver_size, driver_file) != driver_size)
	{
		fclose(driver_file);
		unset_gsf_driver();
		return false;
	}

	fclose(driver_file);
	return true;
}

void Saptapper::unset_gsf_driver(void)
{
	if (manual_gsf_driver != NULL)
	{
		delete [] manual_gsf_driver;
		manual_gsf_driver = NULL;
	}
	manual_gsf_driver_size = 0;
	manual_minigsf_offset = GSF_INVALID_OFFSET;
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

	scan_main_ptr();

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

	scan_main_ptr();

	return true;
}

void Saptapper::scan_main_ptr(void)
{
	rom_main_ptr_offset = GSF_INVALID_OFFSET;

	uint32_t first_instruction = mget4l(&rom[0]);
	if (rom_size < 4 || !is_arm_branch(first_instruction)) {
		return;
	}

	uint32_t offset_start_function = get_arm_branch_destination(0x08000000, first_instruction) & 0x1FFFFFF;

	// typical startup routine:
	// MOV             R0, #0x12
	// MSR             CPSR_cf, R0
	// LDR             SP, =unk_3007FA0
	// MOV             R0, #0x1F
	// MSR             CPSR_cf, R0
	// LDR             SP, =unk_3007F00
	// LDR             R1, =dword_3007FFC
	// ADR             R0, loc_8000104 ; argc
	// STR             R0, [R1]
	// LDR             R1, =(main+1) ; argv
	// MOV             LR, PC
	// BX              R1 ; main
	// B               start
	BytePattern ptn_start(
		"\x12\x00\xa0\xe3\x00\xf0\x29\xe1"
		"\x28\xd0\x9f\xe5\x1f\x00\xa0\xe3"
		"\x00\xf0\x29\xe1\x18\xd0\x9f\xe5"
		"\x1c\x10\x9f\xe5\x20\x00\x8f\xe2"
		"\x00\x00\x81\xe5\x14\x10\x9f\xe5"
		"\x0f\xe0\xa0\xe1\x11\xff\x2f\xe1"
		"\xf2\xff\xff\xea"
		,
		"???x???x"
		"???x???x"
		"???x???x"
		"???x???x"
		"???x???x"
		"???x???x"
		"xxxx"
		,
		52);

	if (offset_start_function + ptn_start.length() > rom_size) {
		return;
	}

	if (!ptn_start.match(&rom[offset_start_function], rom_size - offset_start_function)) {
		return;
	}

	rom_main_ptr_offset = offset_start_function + 0x24 + 8 + (mget4l(&rom[offset_start_function + 0x24]) & 0xFFF);
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

void Saptapper::create_rom_backup(const uint8_t * rom, size_t rom_size)
{
	uninstall_driver();

	rom_backup = new uint8_t[rom_size];
	rom_backup_size = rom_size;

	memcpy(rom_backup, rom, rom_backup_size);
}

void Saptapper::restore_rom_from_backup(uint8_t * rom, size_t rom_size)
{
	memcpy(rom, rom_backup, rom_backup_size);
	rom_size = rom_backup_size;
}

void Saptapper::destroy_rom_backup(uint8_t * & rom_backup, size_t & rom_backup_size)
{
	if (rom_backup != NULL) {
		delete[] rom_backup;
		rom_backup = NULL;
		rom_backup_size = 0;
	}
}

bool Saptapper::install_driver(const uint8_t* driver_block, uint32_t offset, size_t size, bool thumb, bool use_main)
{
	uninstall_driver();

	// check range, alignments, etc.
	if (driver_block == NULL || (offset % 4) != 0 || size == 0 ||
		(offset + size) > rom_size || rom == NULL || rom_size < 4)
	{
		return false;
	}

	if (use_main)
	{
		if (rom_main_ptr_offset == GSF_INVALID_OFFSET)
		{
			return false;
		}
	}
	else
	{
		if (thumb)
		{
			return false;
		}
	}

	// make a backup of unpatched ROM
	create_rom_backup(rom, rom_size);

	// patch ROM
	if (use_main)
	{
		// main function address fetched by LDR
		mput4l(gba_offset_to_address(offset) | (thumb ? 1 : 0), &rom[rom_main_ptr_offset]);
	}
	else
	{
		// ARM B instruction
		mput4l(make_arm_branch(0x08000000, gba_offset_to_address(offset)), &rom[0]);
	}
	memcpy(&rom[offset], driver_block, size);

	return true;
}

void Saptapper::uninstall_driver(void)
{
	if (rom_backup == NULL) {
		return;
	}

	restore_rom_from_backup(rom, rom_size);
	destroy_rom_backup(rom_backup, rom_backup_size);
}

void Saptapper::print_driver_params(const std::map<std::string, VgmDriverParam>& driver_params) const
{
	for (std::map<std::string, VgmDriverParam>::const_iterator it = driver_params.begin(); it != driver_params.end(); ++it)
	{
		std::string name = it->first;
		const VgmDriverParam& data = it->second;

		if (!name.empty() && name[0] == '_')
		{
			name = name.substr(1);
		}

		printf("- %s = %s\n", name.c_str(), data.tostring().c_str());
	}
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

VgmDriver * Saptapper::make_gsflib(const std::string& gsf_path, bool prefer_gba_rom)
{
	std::map<std::string, VgmDriverParam> driver_params;
	return make_gsflib(gsf_path, prefer_gba_rom, driver_params, NULL);
}

VgmDriver * Saptapper::make_gsflib(const std::string& gsf_path, bool prefer_gba_rom, std::map<std::string, VgmDriverParam>& driver_params, uint32_t * ptr_driver_offset)
{
	char msg[2048];
	VgmDriver * driver = NULL;

	// erase error message
	m_message = "";

	// set main function pointer address
	if (rom_main_ptr_offset != GSF_INVALID_OFFSET) {
		if (driver_params.count("ptr_main") == 0) {
			driver_params["ptr_main"] = VgmDriverParam(gba_offset_to_address(rom_main_ptr_offset), true);
		}
	}

	if (driver_params.count("ptr_main") != 0) {
		if (driver_params.count("main") == 0) {
			driver_params["main"] = VgmDriverParam(mget4l(&rom[gba_address_to_offset(driver_params["ptr_main"].getInteger())]) & ~1, true);
		}
	}

	// determine the driver
	if (manual_gsf_driver == NULL)
	{
		for (std::vector<VgmDriver*>::iterator it = drivers.begin(); it != drivers.end(); ++it)
		{
			VgmDriver * driver_candidate = (*it);

			std::map<std::string, VgmDriverParam> driver_params_attempt(driver_params);
			driver_candidate->FindDriverParams(rom, rom_size, driver_params_attempt);
			if (!driver_candidate->ValidateDriverParams(rom, rom_size, driver_params_attempt))
			{
				if (!m_message.empty())
				{
					m_message += "\n";
				}
				m_message += driver_candidate->GetName() + ": " + driver_candidate->message();
				continue;
			}

			driver = driver_candidate;
			driver_params = driver_params_attempt;
			break;
		}

		if (driver == NULL)
		{
			return NULL;
		}
	}

	// determine driver size
	size_t gsf_driver_size;
	if (manual_gsf_driver == NULL)
	{
		gsf_driver_size = driver->GetDriverSize(driver_params);
	}
	else
	{
		gsf_driver_size = manual_gsf_driver_size;
	}

	// determine gsf driver offset
	size_t driver_min_free_space = 0x200;
	size_t driver_margin_size = (gsf_driver_size < driver_min_free_space) ?
		(driver_min_free_space - gsf_driver_size) : 0;
	uint32_t offset_gsf_driver = GSF_INVALID_OFFSET;
	if (manual_gsf_driver_offset == GSF_INVALID_OFFSET)
	{
		// auto-search free block
		offset_gsf_driver = find_free_space(gsf_driver_size + driver_margin_size);
		if (offset_gsf_driver == GSF_INVALID_OFFSET)
		{
			driver_margin_size = 0;
			offset_gsf_driver = find_free_space(gsf_driver_size);
		}

		// add margin size
		if (offset_gsf_driver != GSF_INVALID_OFFSET)
		{
			offset_gsf_driver = (uint32_t) (offset_gsf_driver + driver_margin_size);
		}
	}
	else
	{
		// use specified offset, check the address range
		if (manual_gsf_driver_offset + gsf_driver_size <= rom_size)
		{
			offset_gsf_driver = manual_gsf_driver_offset;
		}
		else
		{
			offset_gsf_driver = GSF_INVALID_OFFSET;
		}
	}

	// check gsf driver offset
	if (offset_gsf_driver == GSF_INVALID_OFFSET)
	{
		sprintf(msg, "Could not find a free space for relocatable driver block (%u bytes)", gsf_driver_size);
		m_message = msg;
		return NULL;
	}
	// set driver offset to given variable
	if (ptr_driver_offset != NULL)
	{
		*ptr_driver_offset = offset_gsf_driver;
	}

	// install driver temporarily
	if (manual_gsf_driver == NULL)
	{
		create_rom_backup(rom, rom_size);

		if (!driver->InstallDriver(rom, rom_size, offset_gsf_driver, driver_params))
		{
			m_message = driver->message();
			uninstall_driver();
			return NULL;
		}
	}
	else
	{
		install_driver(manual_gsf_driver, offset_gsf_driver, gsf_driver_size, manual_driver_is_thumb, manual_driver_is_main);
	}

	// create gba/gsflib file
	put_gsf_exe_header(rom_exe, GBA_ENTRYPOINT, GBA_ENTRYPOINT, (uint32_t) rom_size);
	if (prefer_gba_rom)
	{
		FILE* gba_file = NULL;
		bool write_gba_succeeded = false;

		gba_file = fopen(gsf_path.c_str(), "wb");
		if (gba_file != NULL)
		{
			if (fwrite(rom, 1, rom_size, gba_file) == rom_size)
			{
				write_gba_succeeded = true;
			}
			fclose(gba_file);
		}

		if (!write_gba_succeeded)
		{
			sprintf(msg, "%s - File write error", gsf_path.c_str());
			m_message = msg;
			uninstall_driver();
			return NULL;
		}
	}
	else
	{
		if (!exe2gsf(gsf_path, rom_exe, GSF_EXE_HEADER_SIZE + rom_size))
		{
			sprintf(msg, "%s - File write error", gsf_path.c_str());
			m_message = msg;
			uninstall_driver();
			return NULL;
		}
	}

	// uninstall driver block
	uninstall_driver();

	return driver;
}

bool Saptapper::make_gsf_set(const std::string& rom_path, bool prefer_gba_rom)
{
	std::map<std::string, VgmDriverParam> driver_params;
	return make_gsf_set(rom_path, prefer_gba_rom, driver_params);
}

bool Saptapper::make_gsf_set(const std::string& rom_path, bool prefer_gba_rom, std::map<std::string, VgmDriverParam>& driver_params)
{
	char msg[2048];
	bool result = false;
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
	std::string gsf_dir = rom_dir + (prefer_gba_rom ? "" : rom_basename);
	std::string gsflib_name = rom_basename + (prefer_gba_rom ? ".gsflib.gba" : ".gsflib");
	std::string gsflib_path = (gsf_dir.empty() ? "" : gsf_dir + PATH_SEPARATOR_STR) + gsflib_name;

	if (!quiet)
	{
		printf("%s\n", c_rom_base);
		printf("----------------------------------------\n");
		printf("\n");
	}

	// load ROM image
	if (!load_rom_file(rom_path))
	{
		sprintf(msg, "%s - Could not be loaded", rom_path.c_str());
		m_message = msg;
		fprintf(stderr, "%s\n", m_message.c_str());
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
	if (!gsf_dir.empty())
	{
		_mkdir(gsf_dir.c_str());
		if (!path_isdir(gsf_dir.c_str()))
		{
			sprintf(msg, "%s - Directory could not be created\n", gsf_dir.c_str());
			m_message = msg;
			fprintf(stderr, "%s\n", m_message.c_str());
			close_rom();
			return false;
		}
	}

	// create gsflib
	uint32_t offset_gsf_driver;
	std::string gsflib_error;
	VgmDriver * driver = NULL;
	if (manual_gsf_driver == NULL)
	{
		driver = make_gsflib(gsflib_path, prefer_gba_rom, driver_params, &offset_gsf_driver);
		if (driver == NULL)
		{
			m_message = rom_path + " - " + "No driver matches\n" + message();
			fprintf(stderr, "%s\n", m_message.c_str());
			_rmdir(gsf_dir.c_str());
			close_rom();
			return false;
		}
	}
	else
	{
		make_gsflib(gsflib_path, prefer_gba_rom, driver_params, &offset_gsf_driver);
	}

	// show driver params
	if (!quiet && manual_gsf_driver == NULL)
	{
		print_driver_params(driver_params);
	}

	// determine minigsf offset
	uint32_t offset_minigsf_number;
	if (manual_minigsf_offset == GSF_INVALID_OFFSET)
	{
		offset_minigsf_number = offset_gsf_driver + driver->GetMinixsfOffset(driver_params);
	}
	else
	{
		offset_minigsf_number = (uint32_t) (offset_gsf_driver + manual_minigsf_offset);
	}

	// determine minigsf count
	unsigned int minigsfcount = 0;
	if (manual_minigsf_count == GSF_INVALID_OFFSET)
	{
		minigsfcount = driver->GetSongCount(rom, rom_size, driver_params);
		if (minigsfcount == 0) {
			// set default song count
			minigsfcount = 256;
		}
	}
	else
	{
		minigsfcount = manual_minigsf_count;
	}

	// determine minigsf size
	size_t minigsfsize = 0;
	if (minigsfcount != 0)
	{
		do
		{
			minigsfsize++;
		} while (minigsfsize < 4 && ((minigsfcount - 1) >> (minigsfsize * 8)) != 0);
	}

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

	unsigned int minigsferrors = 0;
	unsigned int minigsfdupes = 0;

	result = true;
	if (!prefer_gba_rom)
	{
		// create minigsfs
		for (unsigned int minigsfindex = 0; minigsfindex < minigsfcount; minigsfindex++)
		{
			char minigsfname[PATH_MAX];

			sprintf(minigsfname, "%s.%04d.minigsf", rom_basename.c_str(), minigsfindex);
			std::string minigsf_path = gsf_dir + PATH_SEPARATOR_STR + minigsfname;

			if (driver == NULL || !driver->IsSongDuplicate(rom, rom_size, driver_params, minigsfindex))
			{
				if (!make_minigsf(minigsf_path, gba_offset_to_address(offset_minigsf_number), minigsfsize, minigsfindex, tags))
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
	}

	close_rom();
	return result;
}

void printUsage(const char *cmd)
{
	const char *availableOptions[] = {
		"--help", "Show this help",
		"-q, --quiet", "Do not output ripping info to STDOUT",
		"-r", "Output uncompressed GBA ROM",
		"-n [count]", "Set minigsf count",
		"-fd, --gsf-driver-file [driver.bin] [0xXXXX] [arm|thumb] [start|main]", "Specify relocatable GSF driver block and minigsf offset",
		"-od, --offset-gsf-driver [0xXXXXXXXX]", "Specify the offset of GSF driver block",
		"-os, --offset-selectsong [0xXXXXXXXX]", "Specify the offset of sappy_selectsong function",
		"-ot, --offset-songtable [0xXXXXXXXX]", "Specify the offset of song table (well known Sappy offset)",
		"-om, --offset-main [0xXXXXXXXX]", "Specify the offset of sappy_main function",
		"-oi, --offset-init [0xXXXXXXXX]", "Specify the offset of sappy_init function",
		"-ov, --offset-vsync [0xXXXXXXXX]", "Specify the offset of sappy_vsync function",
		"--tag-gsfby [name]", "Specify the nickname of GSF ripper",
		"--find-freespace [ROM.gba] [size]", "Find free space and quit",
		"--rom2gsf [GBA ROM file] [-m]", "Convert GBA ROM into GSF (-m for multiboot ROM)",
		"--minigsf [basename] [offset] [size] [count]", "Create minigsf files",
	};

	printf("%s %s\n", APP_NAME, APP_VER);
	printf("======================\n");
	printf("\n");
	printf("%s. Created by %s.\n", APP_DESC, APP_AUTHOR);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s <GBA Files>`\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");

	for (int i = 0; i < sizeof(availableOptions) / sizeof(availableOptions[0]); i += 2)
	{
		printf("`%s`\n", availableOptions[i]);
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

	bool prefer_gba_rom = false;
	std::map<std::string, VgmDriverParam> user_driver_params;

	app.set_quiet(false);

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "-q") == 0 || strcmp(argv[argi], "--quiet") == 0)
		{
			app.set_quiet(true);
		}
		else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0)
		{
			// for backward compatibility, for now
			app.set_quiet(false);
		}
		else if (strcmp(argv[argi], "-r") == 0)
		{
			prefer_gba_rom = true;
		}
		else if (strcmp(argv[argi], "-n") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			ul = strtoul(argv[argi + 1], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			app.set_minigsf_count(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "-fd") == 0 || strcmp(argv[argi], "--gsf-driver-file") == 0)
		{
			if (argi + 4 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			ul = strtoul(argv[argi + 2], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			bool thumb;
			if (strcmp(argv[argi + 3], "arm") == 0) {
				thumb = false;
			}
			else if (strcmp(argv[argi + 3], "thumb") == 0) {
				thumb = true;
			}
			else {
				fprintf(stderr, "Error: ARM/THUMB type error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			bool use_main;
			if (strcmp(argv[argi + 4], "start") == 0) {
				use_main = false;
			}
			else if (strcmp(argv[argi + 4], "main") == 0) {
				use_main = true;
			}
			else {
				fprintf(stderr, "Error: Entrypoint selection error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			if (!app.set_gsf_driver_file(argv[argi + 1], ul, thumb, use_main))
			{
				fprintf(stderr, "Error: Unable to set custom driver \"%s\"\n", argv[argi + 1]);
				return EXIT_FAILURE;
			}
			argi += 4;
		}
		else if (strcmp(argv[argi], "-od") == 0 || strcmp(argv[argi], "--offset-gsf-driver") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			app.set_gsf_driver_offset(ul);
			argi++;
		}
		else if (strcmp(argv[argi], "-os") == 0 || strcmp(argv[argi], "--offset-selectsong") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			user_driver_params["sub_selectsong"] = VgmDriverParam(ul, true);
			argi++;
		}
		else if (strcmp(argv[argi], "-ot") == 0 || strcmp(argv[argi], "--offset-songtable") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			user_driver_params["_array_songs"] = VgmDriverParam(ul, true);
			argi++;
		}
		else if (strcmp(argv[argi], "-om") == 0 || strcmp(argv[argi], "--offset-main") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			user_driver_params["sub_main"] = VgmDriverParam(ul, true);
			argi++;
		}
		else if (strcmp(argv[argi], "-oi") == 0 || strcmp(argv[argi], "--offset-init") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			user_driver_params["sub_init"] = VgmDriverParam(ul, true);
			argi++;
		}
		else if (strcmp(argv[argi], "-ov") == 0 || strcmp(argv[argi], "--offset-vsync") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			if (strcmp(argv[argi + 1], "null") == 0)
			{
				ul = 0;
			}
			else
			{
				ul = strtoul(argv[argi + 1], &strtol_endp, 16);
				if (strtol_endp != NULL && *strtol_endp != '\0')
				{
					fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
					return EXIT_FAILURE;
				}
				// GBA ROM offset to address
				if (ul >= 0x0000000 && ul <= 0x1FFFFFF)
				{
					ul |= 0x8000000;
				}
			}
			user_driver_params["sub_vsync"] = VgmDriverParam(ul, true);
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
			printf("0x%08X\n", gba_offset_to_address(offset));

			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[argi], "--largespace") == 0)
		{
			// experimental: it's secret to everybody :)
			app.set_prefer_larger_free_space(true);
		}
		else if (strcmp(argv[argi], "--minigsf") == 0)
		{
			if (argi + 5 != argc)
			{
				fprintf(stderr, "Error: Too few/more arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			std::string minigsf_basename(argv[argi + 1]);

			ul = strtoul(argv[argi + 2], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			uint32_t minigsf_offset = ul;
			if ((minigsf_offset & 0xFE000000) == 0)
			{
				fprintf(stderr, "Error: Not a GBA ROM address 0x%08X\n", minigsf_offset);
				return EXIT_FAILURE;
			}

			ul = strtoul(argv[argi + 3], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			uint32_t minigsf_size = ul;
			if (minigsf_size > 4)
			{
				fprintf(stderr, "Error: Too large minigsf size\n");
				return EXIT_FAILURE;
			}

			ul = strtoul(argv[argi + 4], &strtol_endp, 0);
			if (strtol_endp != NULL && *strtol_endp != '\0')
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}
			uint32_t minigsf_count = ul;
			if (minigsf_size == 0 || (minigsf_size < 4 && ((minigsf_count - 1) >> (minigsf_size * 8)) != 0))
			{
				fprintf(stderr, "Error: Too short minigsf size\n");
				return EXIT_FAILURE;
			}

			// determine gsflib filename
			char gsflib_filename[PATH_MAX];
			sprintf(gsflib_filename, "%s.gsflib", minigsf_basename.c_str());

			// create gsf tag
			std::map<std::string, std::string> tags;
			tags["_lib"] = gsflib_filename;

			// create minigsf files
			bool succeeded = true;
			for (uint32_t minigsf_num = 0; minigsf_num < minigsf_count; minigsf_num++)
			{
				// determine minigsf filename
				char minigsf_filename[PATH_MAX];
				sprintf(minigsf_filename, "%s-%04d.minigsf", minigsf_basename.c_str(), minigsf_num);

				// save minigsf
				if (!Saptapper::make_minigsf(minigsf_filename, minigsf_offset, minigsf_size, minigsf_num, tags))
				{
					fprintf(stderr, "Error: Unable to save \"%s\"\n", minigsf_filename);
					succeeded = false;
				}
			}

			return succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
		}
		else if (strcmp(argv[argi], "--rom2gsf") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: No input files for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			bool succeeded = true;
			for (argi = argi + 1; argi < argc; argi++)
			{
				const char * rom_filename = argv[argi];

				bool multiboot;
				if (argi + 1 < argc && strcmp(argv[argi + 1], "-m") == 0)
				{
					multiboot = true;
					argi++;
				}
				else
				{
					multiboot = false;
				}

				// determine gsf filename
				char gsf_filename[PATH_MAX];
				strcpy(gsf_filename, rom_filename);
				path_stripext(gsf_filename);
				strcat(gsf_filename, ".gsf");

				// get file size
				off_t rom_size_off = path_getfilesize(rom_filename);
				if (rom_size_off < 0)
				{
					fprintf(stderr, "Error: File not found \"%s\"\n", rom_filename);
					succeeded = false;
					continue;
				}
				size_t rom_size = (size_t)rom_size_off;

				// allocate memory for gba rom
				size_t exe_size = GSF_EXE_HEADER_SIZE + rom_size;
				uint8_t * exe = new uint8_t[exe_size];
				if (exe == NULL)
				{
					fprintf(stderr, "Error: Memory allocation error\n");
					succeeded = false;
					break;
				}

				// put gsf header
				uint32_t gsf_entrypoint = multiboot ? 0x02000000 : 0x08000000;
				Saptapper::put_gsf_exe_header(exe, gsf_entrypoint, gsf_entrypoint, (uint32_t)rom_size);

				// open gba rom
				FILE * fp = fopen(rom_filename, "rb");
				if (fp == NULL)
				{
					fprintf(stderr, "Error: File open error \"%s\"\n", rom_filename);
					succeeded = false;

					delete[] exe;
					continue;
				}

				// read gba rom
				if (fread(&exe[GSF_EXE_HEADER_SIZE], 1, rom_size, fp) != rom_size)
				{
					fprintf(stderr, "Error: File read error \"%s\"\n", rom_filename);
					succeeded = false;

					fclose(fp);
					delete[] exe;
					continue;
				}

				fclose(fp);

				// output gsf file
				if (!Saptapper::exe2gsf(gsf_filename, exe, exe_size))
				{
					fprintf(stderr, "Error: Unable to save \"%s\"\n", gsf_filename);
					succeeded = false;

					delete[] exe;
					continue;
				}

				delete[] exe;
			}

			return succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
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
		std::map<std::string, VgmDriverParam> driver_params(user_driver_params);
		result = result && app.make_gsf_set(argv[argi], prefer_gba_rom, driver_params);
	}
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
