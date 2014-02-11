
#ifndef VGMDRIVERPARAM_H_INCLUDED
#define VGMDRIVERPARAM_H_INCLUDED

#include <stdint.h>
#include <memory.h>

#include <iostream>
#include <string>
#include <stdexcept>

enum VgmDriverParamType
{
	INTEGER = 0,
	BOOL_SW,
	STRING,
	BINARY,
};

class VgmDriverParam
{
public:
	VgmDriverParam()
	{
		datatype = STRING;
		data = "";
		display_str = "";
	}

	VgmDriverParam(uint32_t value, bool hexadecimal)
	{
		uint8_t bytes[4];
		char str[64];

		datatype = INTEGER;
		bytes[0] = value & 0xff;
		bytes[1] = (value >> 8) & 0xff;
		bytes[2] = (value >> 16) & 0xff;
		bytes[3] = (value >> 24) & 0xff;
		data = std::string((char *) bytes, 4);

		if (value == 0)
		{
			display_str = "0";
		}
		else if (hexadecimal)
		{
			sprintf(str, "0x%08X", value);
			display_str = str;
		}
		else
		{
			sprintf(str, "%d", *((int32_t *)(&value)));
			display_str = str;
		}
	}

	VgmDriverParam(bool sw)
	{
		uint8_t bytes[1];

		datatype = BOOL_SW;
		bytes[0] = sw ? 1 : 0;
		data = std::string((char *) bytes, 1);

		if (sw)
		{
			display_str = "true";
		}
		else
		{
			display_str = "false";
		}
	}

	VgmDriverParam(const std::string& str)
	{
		datatype = STRING;
		data = str;
		display_str = str;
	}

	VgmDriverParam(const uint8_t * buf, uint32_t size)
	{
		char str[4];

		datatype = BINARY;
		data = std::string((char *) buf, size);

		display_str = "";
		for (uint32_t i = 0; i < size; i++)
		{
			sprintf(str, "%02x", buf[i]);
			display_str += str;
		}
	}

	virtual ~VgmDriverParam()
	{
	}

	operator std::string() const
	{
		return display_str;
	}

	friend std::ostream& operator<<(std::ostream& os, const VgmDriverParam& param);

	uint32_t getInteger() const
	{
		uint32_t value;

		if (datatype != INTEGER)
		{
			throw std::invalid_argument("Parameter type must be integer.");
		}

		value  = ((uint8_t) data[0]);
		value |= ((uint8_t) data[1]) << 8;
		value |= ((uint8_t) data[2]) << 16;
		value |= ((uint8_t) data[3]) << 24;
		return value;
	}

	bool getBoolean() const
	{
		if (datatype != BOOL_SW)
		{
			throw std::invalid_argument("Parameter type must be boolean.");
		}

		return (data[0] != 0);
	}

	const std::string& getString() const
	{
		if (datatype != STRING && datatype != BINARY)
		{
			throw std::invalid_argument("Parameter type must be string or binary.");
		}

		return data;
	}

	void getBinary(uint8_t * buf, uint32_t size) const
	{
		if (datatype != STRING && datatype != BINARY)
		{
			throw std::invalid_argument("Parameter type must be string or binary.");
		}

		if (size > data.size())
		{
			size = (uint32_t) data.size();
		}

		memcpy(buf, data.c_str(), size);
	}

	size_t size() const
	{
		return data.size();
	}

	VgmDriverParamType type() const
	{
		return datatype;
	}

	const std::string& tostring() const
	{
		return display_str;
	}

protected:
	std::string data;
	std::string display_str;
	VgmDriverParamType datatype;
};

#endif
