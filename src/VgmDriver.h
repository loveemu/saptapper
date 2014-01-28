
#ifndef VGMDRIVER_H_INCLUDED
#define VGMDRIVER_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <map>
#include <stdexcept>

#include "VgmDriverParam.h"

class VgmDriver
{
public:
	VgmDriver() {}
	virtual ~VgmDriver() {}

	// Return human-friendly driver name.
	virtual const std::string& GetName() const = 0;

	// Return plugin version.
	virtual const std::string& GetVersion() const = 0;

	// Parse ROM and collect necessary parameters to construct driver block.
	virtual void FindDriverParams(const uint8_t * rom, size_t rom_size, std::map<std::string, VgmDriverParam>& params) = 0;

	// Validate driver parameters and return if driver block is installable or not.
	virtual bool ValidateDriverParams(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params) = 0;

	// Return song count in the ROM. Return 0 if not obtainable.
	virtual int GetSongCount(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params) = 0;

	// Return offset of song index variable in relocatable driver block.
	virtual off_t GetMinixsfOffset(const std::map<std::string, VgmDriverParam>& params) const = 0;

	// Return relocatable driver block size.
	virtual size_t GetDriverSize(const std::map<std::string, VgmDriverParam>& params) const = 0;

	// Install driver block into specified offset.
	virtual bool InstallDriver(uint8_t * rom, size_t rom_size, off_t offset, const std::map<std::string, VgmDriverParam>& params) = 0;

	// Return if the specified song is a duplicate of any previous songs.
	virtual bool IsSongDuplicate(const uint8_t * rom, size_t rom_size, const std::map<std::string, VgmDriverParam>& params, uint32_t song) = 0;

	// Return last error message.
	virtual std::string message() const = 0;
};

/*

Parameter Block
---------------

Driver parameter is a short data that specifies game specific offsets and such.

Detail of parameter depends on derived VgmDriver class, however,
there are some commonly used names as follows.

### Reserved Names ###

Names beginning with an underscore are reserved for special use.

_array_songs
  : Address of a song address table (sort of). Required for determining the number of songs.

### Common Names ###

Following names are not mandatory, but recommended.

sub_init (sub_init_xxxxx, if there are multiple init functions)
  : Address of initialization function.

sub_selectsong
  : Address of song selector function. In most cases, it also starts music playback.

sub_main
  : Address of a sort of "main" function, that is called everytime from main loop.

sub_vsync
  : Address of a vsync callback function.

Use 0 for nullptr.

*/

#endif
