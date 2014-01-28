
#include <stdint.h>
#include <memory.h>

#include <iostream>
#include <string>
#include <stdexcept>

#include "VgmDriverParam.h"

std::ostream& operator<<(std::ostream& os, const VgmDriverParam& param)
{
	os << param.display_str;
	return os;
}
