
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <string>
#include <map>
#include <stdexcept>

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
	uint32_t sub_selectsong = GSF_INVALID_OFFSET;
	if (params.count("sub_selectsong") == 0)
	{
		sub_selectsong = find_selectsong(rom, (uint32_t)rom_size);
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

	uint32_t sub_init = GSF_INVALID_OFFSET;
	if (params.count("sub_init") == 0)
	{
		sub_init = find_init(rom, (uint32_t)rom_size);
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

	uint32_t sub_init_2 = GSF_INVALID_OFFSET;
	uint32_t ofs_init_2_a0 = 0;
	if (params.count("sub_init_2") == 0)
	{
		sub_init_2 = find_init_2(rom, (uint32_t)rom_size, ofs_init_2_a0);
		if (sub_init_2 == GSF_INVALID_OFFSET)
		{
			return;
		}
		params["sub_init_2"] = VgmDriverParam(gba_offset_to_address(sub_init_2), true);
		params["ofs_init_2_a0"] = VgmDriverParam(ofs_init_2_a0, true);
	}
	else
	{
		// TODO: ofs_init_2_a0
		sub_init_2 = params.at("sub_init_2").getInteger();
		if (sub_init_2 == 0)
		{
			sub_init_2 = GSF_INVALID_OFFSET;
		}
		else if (sub_init_2 >= 0x8000000 && sub_init_2 < 0x8000000 + rom_size)
		{
			sub_init_2 = gba_address_to_offset(sub_init_2);
		}
		else
		{
			return;
		}
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

	if (params.count("sub_init") == 0)
	{
		sprintf(str, "sub_init not found");
		m_message = str;
		return false;
	}
	uint32_t sub_init = params.at("sub_init").getInteger();
	if (sub_init != 0 && (sub_init < 0x08000000 || sub_init >= 0x8000000 + rom_size))
	{
		sprintf(str, "sub_init is out of range (0x%08X)", sub_init);
		m_message = str;
		return false;
	}

	if (params.count("sub_init_2") == 0)
	{
		sprintf(str, "sub_init_2 not found");
		m_message = str;
		return false;
	}
	uint32_t sub_init_2 = params.at("sub_init_2").getInteger();
	if (sub_init_2 != 0 && (sub_init_2 < 0x08000000 || sub_init_2 >= 0x8000000 + rom_size))
	{
		sprintf(str, "sub_init_2 is out of range (0x%08X)", sub_init_2);
		m_message = str;
		return false;
	}

	if (params.count("ofs_init_2_a0") == 0)
	{
		sprintf(str, "ofs_init_2_a0 not found");
		m_message = str;
		return false;
	}
	uint32_t ofs_init_2_a0 = params.at("ofs_init_2_a0").getInteger();
	if (params.at("ofs_init_2_a0").type() != VgmDriverParamType::INTEGER)
	{
		sprintf(str, "ofs_init_2_a0 must be an integer");
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

bool NatsumeDriver::GetIfDriverUseMain(void) const
{
	return true;
}

// Install driver block into specified offset.
bool NatsumeDriver::InstallDriver(uint8_t * rom, size_t rom_size, off_t offset, const std::map<std::string, VgmDriverParam>& params)
{
	char str[512];
	uint32_t ptr_main = params.at("ptr_main").getInteger();
	uint32_t sub_init = params.at("sub_init").getInteger();
	uint32_t ofs_init_2_a0 = params.at("ofs_init_2_a0").getInteger();
	uint32_t sub_init_2 = params.at("sub_init_2").getInteger();
	uint32_t sub_selectsong = params.at("sub_selectsong").getInteger();

	size_t driver_size = GetDriverSize(params);
	if (offset < 0 || (offset % 4) != 0 || (size_t) offset + driver_size > rom_size)
	{
		sprintf(str, "Could not install driver at 0x%08X (invalid offset)", offset);
		m_message = str;
		return false;
	}

	// install driver
	mput4l(sub_init | (sub_init != 0 ? 1 : 0), &driver_block[DRIVER_PARAM_BASE]);
	mput4l(sub_init_2 | (sub_init_2 != 0 ? 1 : 0), &driver_block[DRIVER_PARAM_BASE + 4]);
	mput4l(ofs_init_2_a0, &driver_block[DRIVER_PARAM_BASE + 8]);
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
	if (params.count("_array_songs") == 0)
	{
		return false;
	}
	uint32_t array_songs = params.at("_array_songs").getInteger();
	if (array_songs < 0x08000000 || array_songs >= 0x8000000 + rom_size)
	{
		return false;
	}
	return false;
}

uint32_t NatsumeDriver::find_selectsong(const uint8_t * rom, uint32_t rom_size)
{
	if (memcmp(&rom[0xac], "A8EJ", 4) != 0) {
		return GSF_INVALID_OFFSET;
	}

	return 0x08069E94;
}

uint32_t NatsumeDriver::find_init(const uint8_t * rom, uint32_t rom_size)
{
	if (memcmp(&rom[0xac], "A8EJ", 4) != 0) {
		return GSF_INVALID_OFFSET;
	}

	return 0x080005A8;
}

uint32_t NatsumeDriver::find_init_2(const uint8_t * rom, uint32_t rom_size, uint32_t & a0)
{
	if (memcmp(&rom[0xac], "A8EJ", 4) != 0) {
		return GSF_INVALID_OFFSET;
	}

	a0 = 0x03000160;
	return 0x080006D0;
}
