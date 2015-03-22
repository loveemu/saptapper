
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <string>
#include <map>
#include <stdexcept>
#include <algorithm>

#include "NatsumeDriver.h"
#include "BytePattern.h"
#include "cbyteio.h"
#include "saptapper.h"

#include "asm/natsume-gsf.h"

#define DRIVER_PARAM_BASE	0x1C

NatsumeDriver::NatsumeDriver() :
	m_name("Natsume"),
	m_version("1.0")
{
}

NatsumeDriver::~NatsumeDriver()
{
}

// Return human-friendly driver name.
const std::string& NatsumeDriver::GetName() const
{
	return m_name;
}

// Return class version.
const std::string& NatsumeDriver::GetVersion() const
{
	return m_version;
}

// Parse ROM and collect necessary parameters to construct driver block.
void NatsumeDriver::FindDriverParams(const uint8_t * rom, size_t rom_size, std::map<std::string, VgmDriverParam>& params)
{
	if (params.count("main") == 0) {
		return;
	}

	uint32_t offset_main_ptr = gba_address_to_offset(params["main"].getInteger());

	// PUSH    {R4-R6,LR}
	// LDR     R1, =0x4000204
	// LDR     R2, =0x4014
	// MOVS    R0, R2
	// STRH    R0, [R1]
	// BL      sub_8000624
	// BL      sub_8001E3C
	// BL      InitIRQHandlers
	// MOVS    R1, #0x4000000
	// MOVS    R0, #0x60 ; '`'
	// STRH    R0, [R1]
	// LDR     R1, =0x3000C90
	// MOVS    R0, #1
	// STR     R0, [R1]
	// LDR     R0, =0x3001A04
	// MOVS    R1, #0
	// STR     R1, [R0]
	// LDR     R0, =0x3000830
	// STR     R1, [R0]
	// BL      sub_8037A30
	// LDR     R0, =0x3000160  ; start address of sound region
	// BL      InitSoundRegion
	BytePattern ptn_main(
		"\x70\xb5\x20\x49\x20\x4a\x10\x1c"
		"\x08\x80\x00\xf0\x8d\xf8\x01\xf0"
		"\x97\xfc\x00\xf0\x4b\xf8\x80\x21"
		"\xc9\x04\x60\x20\x08\x80\x1b\x49"
		"\x01\x20\x08\x60\x1a\x48\x00\x21"
		"\x01\x60\x1a\x48\x01\x60\x37\xf0"
		"\x81\xfa\x19\x48\x00\xf0\xce\xf8"
		,
		"?x?x?x?x"
		"?x??????"
		"???????x"
		"?x?x?x?x"
		"?x?x?x?x"
		"?x?x?x??"
		"???x????"
		,
		0x38);

	if (!ptn_main.match(rom, rom_size, offset_main_ptr)) {
		return;
	}

	uint32_t sub_init_irq = get_thumb_branch_with_link_destination(gba_offset_to_address(offset_main_ptr + 0x12), mget4l(&rom[offset_main_ptr + 0x12]));
	uint32_t sub_init_sound = get_thumb_branch_with_link_destination(gba_offset_to_address(offset_main_ptr + 0x34), mget4l(&rom[offset_main_ptr + 0x34]));
	uint32_t ofs_sound_work_imm = ((offset_main_ptr + 0x32 + 4) & ~3) + (rom[offset_main_ptr + 0x32] * 4);
	uint32_t ofs_sound_work = mget4l(&rom[ofs_sound_work_imm]);

	if (params.count("sub_init_irq") == 0) {
		params["sub_init_irq"] = VgmDriverParam(sub_init_irq, true);
	}

	if (params.count("sub_init_sound") == 0) {
		params["sub_init_sound"] = VgmDriverParam(sub_init_sound, true);
	}

	if (params.count("ofs_sound_work") == 0) {
		params["ofs_sound_work"] = VgmDriverParam(ofs_sound_work, true);
	}

	// PUSH    {LR}
	// BL      sub_80677D4     ; zero clear etc.
	// LDR     R0, =0x807EC8C
	// BL      sub_8069DC0     ; start set addresses
	// LDR     R0, =0x807EFAC
	// BL      sub_8069FDC
	// LDR     R0, =0x807F9E4
	// BL      sub_806A1E8
	// LDR     R0, =0x807F840
	// BL      sub_8069064
	// LDR     R0, =0x807F978
	// BL      sub_8069078     ; end set addresses
	// MOVS    R0, #0
	// BL      SelectSong
	// POP     {R0}
	// BX      R0
	BytePattern ptn_init_sound(
		"\x00\xb5\x67\xf0\x7f\xf8\x0a\x48"
		"\x69\xf0\x72\xfb\x09\x48\x69\xf0"
		"\x7d\xfc\x09\x48\x69\xf0\x80\xfd"
		"\x08\x48\x68\xf0\xbb\xfc\x08\x48"
		"\x68\xf0\xc2\xfc\x00\x20\x69\xf0"
		"\xcd\xfb\x01\xbc\x00\x47"
		,
		"xx?????x"
		"?????x??"
		"???x????"
		"?x?????x"
		"????xx??"
		"??xxxx"
		,
		0x2e);

	uint32_t offset_sub_init_sound = gba_address_to_offset(sub_init_sound);
	if (!ptn_init_sound.match(rom, rom_size, offset_sub_init_sound)) {
		return;
	}

	uint32_t sub_selectsong = get_thumb_branch_with_link_destination(gba_offset_to_address(offset_sub_init_sound + 0x26), mget4l(&rom[offset_sub_init_sound + 0x26]));
	if (params.count("sub_selectsong") == 0) {
		params["sub_selectsong"] = VgmDriverParam(sub_selectsong, true);
	}
}

// Validate driver parameters and return if driver block is installable or not.
bool NatsumeDriver::ValidateDriverParams(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params)
{
	char str[512];

	if (params.count("ptr_main") == 0)
	{
		sprintf(str, "ptr_main not found");
		m_message = str;
		return false;
	}
	uint32_t ptr_main = params.at("ptr_main").getInteger();
	if (ptr_main != 0 && (ptr_main < 0x08000000 || ptr_main >= 0x8000000 + rom_size))
	{
		sprintf(str, "ptr_main is out of range (0x%08X)", ptr_main);
		m_message = str;
		return false;
	}

	if (params.count("sub_selectsong") == 0)
	{
		sprintf(str, "sub_selectsong not found");
		m_message = str;
		return false;
	}
	uint32_t sub_selectsong = params.at("sub_selectsong").getInteger();
	if (sub_selectsong != 0 && (sub_selectsong < 0x08000000 || sub_selectsong >= 0x8000000 + rom_size))
	{
		sprintf(str, "sub_selectsong is out of range (0x%08X)", sub_selectsong);
		m_message = str;
		return false;
	}

	if (params.count("sub_init_irq") == 0)
	{
		sprintf(str, "sub_init_irq not found");
		m_message = str;
		return false;
	}
	uint32_t sub_init_irq = params.at("sub_init_irq").getInteger();
	if (sub_init_irq != 0 && (sub_init_irq < 0x08000000 || sub_init_irq >= 0x8000000 + rom_size))
	{
		sprintf(str, "sub_init_irq is out of range (0x%08X)", sub_init_irq);
		m_message = str;
		return false;
	}

	if (params.count("sub_init_sound") == 0)
	{
		sprintf(str, "sub_init_sound not found");
		m_message = str;
		return false;
	}
	uint32_t sub_init_sound = params.at("sub_init_sound").getInteger();
	if (sub_init_sound != 0 && (sub_init_sound < 0x08000000 || sub_init_sound >= 0x8000000 + rom_size))
	{
		sprintf(str, "sub_init_sound is out of range (0x%08X)", sub_init_sound);
		m_message = str;
		return false;
	}

	if (params.count("ofs_sound_work") == 0)
	{
		sprintf(str, "ofs_sound_work not found");
		m_message = str;
		return false;
	}
	uint32_t ofs_sound_work = params.at("ofs_sound_work").getInteger();
	if (params.at("ofs_sound_work").type() != VgmDriverParamType::INTEGER)
	{
		sprintf(str, "ofs_sound_work must be an integer");
		m_message = str;
		return false;
	}

	return true;
}

// Return song count in the ROM. Return 0 if not obtainable.
int NatsumeDriver::GetSongCount(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params)
{
	return 0;
}

// Return offset of song index variable in relocatable driver block.
off_t NatsumeDriver::GetMinixsfOffset(const std::map<std::string, VgmDriverParam>& params) const
{
	return DRIVER_PARAM_BASE + 16;
}

// Return relocatable driver block size.
size_t NatsumeDriver::GetDriverSize(const std::map<std::string, VgmDriverParam>& params) const
{
	return sizeof(driver_block);
}

// Install driver block into specified offset.
bool NatsumeDriver::InstallDriver(uint8_t * rom, size_t rom_size, off_t offset, const std::map<std::string, VgmDriverParam>& params)
{
	char str[512];
	uint32_t ptr_main = params.at("ptr_main").getInteger();
	uint32_t sub_init_irq = params.at("sub_init_irq").getInteger();
	uint32_t ofs_sound_work = params.at("ofs_sound_work").getInteger();
	uint32_t sub_init_sound = params.at("sub_init_sound").getInteger();
	uint32_t sub_selectsong = params.at("sub_selectsong").getInteger();

	size_t driver_size = GetDriverSize(params);
	if (offset < 0 || (offset % 4) != 0 || (size_t) offset + driver_size > rom_size)
	{
		sprintf(str, "Could not install driver at 0x%08X (invalid offset)", offset);
		m_message = str;
		return false;
	}

	// install driver
	mput4l(sub_init_irq | (sub_init_irq != 0 ? 1 : 0), &driver_block[DRIVER_PARAM_BASE]);
	mput4l(sub_init_sound | (sub_init_sound != 0 ? 1 : 0), &driver_block[DRIVER_PARAM_BASE + 4]);
	mput4l(ofs_sound_work, &driver_block[DRIVER_PARAM_BASE + 8]);
	mput4l(sub_selectsong | (sub_selectsong != 0 ? 1 : 0), &driver_block[DRIVER_PARAM_BASE + 12]);
	memcpy(&rom[offset], driver_block, driver_size);

	// modify entrypoint
	uint32_t offset_main_ptr = gba_address_to_offset(ptr_main);
	mput4l(gba_offset_to_address(offset) | 1, &rom[offset_main_ptr]);

	return true;
}

// Return if the specified song is a duplicate of any previous songs.
bool NatsumeDriver::IsSongDuplicate(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params, uint32_t song)
{
	return false;
}
