
#ifndef NATSUMEDRIVER_H_INCLUDED
#define NATSUMEDRIVER_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <map>
#include <stdexcept>

#include "VgmDriver.h"

class NatsumeDriver : public VgmDriver
{
public:
	NatsumeDriver();
	virtual ~NatsumeDriver();

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

protected:
	std::string m_name;
	std::string m_version;
	std::string m_message;
};

#endif
