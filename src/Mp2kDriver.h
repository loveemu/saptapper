
#ifndef MP2KDRIVER_H_INCLUDED
#define MP2KDRIVER_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <map>
#include <stdexcept>

#include "VgmDriver.h"

class Mp2kDriver : public VgmDriver
{
public:
	Mp2kDriver();
	virtual ~Mp2kDriver();

	// Return human-friendly driver name.
	virtual const std::string& GetName() const;

	// Return class version.
	virtual const std::string& GetVersion() const;

	// Parse ROM and collect necessary parameters to construct driver block.
	virtual void FindDriverParams(const uint8_t * rom, size_t rom_size, std::map<std::string, VgmDriverParam>& params);

	// Validate driver parameters and return if driver block is installable or not.
	virtual bool ValidateDriverParams(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params);

	// Return song count in the ROM. Return 0 if not obtainable.
	virtual int GetSongCount(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params);

	// Return offset of song index variable in relocatable driver block.
	virtual off_t GetMinixsfOffset(const std::map<std::string, VgmDriverParam>& params) const;

	// Return relocatable driver block size.
	virtual size_t GetDriverSize(const std::map<std::string, VgmDriverParam>& params) const;

	// Install driver block into specified offset.
	virtual bool InstallDriver(uint8_t * rom, size_t rom_size, off_t offset, const std::map<std::string, VgmDriverParam>& params);

	// Return if the specified song is a duplicate of any previous songs.
	virtual bool IsSongDuplicate(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params, uint32_t song);

	// Return last error message.
	virtual std::string message() const
	{
		return m_message;
	}

	static uint32_t find_m4a_selectsong(const uint8_t * rom, uint32_t rom_size);
	static uint32_t find_m4a_songtable(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_selectsong);
	static uint32_t find_m4a_main(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_selectsong);
	static uint32_t find_m4a_init(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_main);
	static uint32_t find_m4a_vsync(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_init);
	static unsigned int m4a_get_song_count(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_songtable);
	static bool m4a_is_song_duplicate(const uint8_t * rom, uint32_t rom_size, uint32_t offset_m4a_songtable, unsigned int song_index);

protected:
	std::string m_name;
	std::string m_version;
	std::string m_message;
};

#endif
