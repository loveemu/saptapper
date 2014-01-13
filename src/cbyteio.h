/**
 * Inline byte I/O routines for C.
 */

#ifndef CBYTEIO_H_INCLUDED
#define CBYTEIO_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#ifndef __cplusplus
#ifdef HAVE_STDBOOL
#include <stdbool.h>
#else
#ifndef bool
typedef int bool;
#define true    1
#define false   0
#endif /* bool */
#endif /* HAVE_STDBOOL */
#endif /* C++ */

#ifndef INLINE
#ifdef inline
#define INLINE  inline
#elsif defined(__inline)
#define INLINE  __inline
#else
#define INLINE
#endif
#endif /* !INLINE */

/** unsigned byte to signed */
static INLINE int utos1(unsigned int value)
{
	return (value & 0x80) ? -(signed)(value ^ 0xff)-1 : value;
}

/** unsigned 2 bytes to signed */
static INLINE int utos2(unsigned int value)
{
	return (value & 0x8000) ? -(signed)(value ^ 0xffff)-1 : value;
}

/** unsigned 3 bytes to signed */
static INLINE int utos3(unsigned int value)
{
	return (value & 0x800000) ? -(signed)(value ^ 0xffffff)-1 : value;
}

/** unsigned 4 bytes to signed */
static INLINE int utos4(unsigned int value)
{
	return (value & 0x80000000) ? -(signed)(value ^ 0xffffffff)-1 : value;
}

/** get length of variable-length integer */
static INLINE int varintlen(unsigned int value)
{
	int len = 0;
	do
	{
		value >>= 7;
		len++;
	} while (len < 4 && value != 0);
	return len;
}

/** get 1 byte */
static INLINE unsigned int mget1(const uint8_t* data)
{
	return data[0];
}

/** get 2 bytes (little-endian) */
static INLINE unsigned int mget2l(const uint8_t* data)
{
	return data[0] | (data[1] * 0x0100);
}

/** get 3 bytes (little-endian) */
static INLINE unsigned int mget3l(const uint8_t* data)
{
	return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000);
}

/** get 4 bytes (little-endian) */
static INLINE unsigned int mget4l(const uint8_t* data)
{
	return data[0] | (data[1] * 0x0100) | (data[2] * 0x010000) | (data[3] * 0x01000000);
}

/** get variable-length integer (little-endian) */
static INLINE unsigned int mgetvl(const uint8_t* data)
{
	unsigned int value = 0;
	int len = 0;
	uint8_t c;
	do
	{
		c = data[len];
		value |= (c & 0x7F) << (7 * len);
	} while (len < 4 && (c & 0x80) != 0);
	return value;
}

/** get 2 bytes (big-endian) */
static INLINE unsigned int mget2b(const uint8_t* data)
{
	return data[1] | (data[0] * 0x0100);
}

/** get 3 bytes (big-endian) */
static INLINE unsigned int mget3b(const uint8_t* data)
{
	return data[2] | (data[1] * 0x0100) | (data[0] * 0x010000);
}

/** get 4 bytes (big-endian) */
static INLINE unsigned int mget4b(const uint8_t* data)
{
	return data[3] | (data[2] * 0x0100) | (data[1] * 0x010000) | (data[0] * 0x01000000);
}

/** get variable-length integer (big-endian) */
static INLINE unsigned int mgetvb(const uint8_t* data)
{
	unsigned int value = 0;
	int len = 0;
	uint8_t c;
	do
	{
		c = data[len];
		value = (value << 7) | (c & 0x7F);
	} while (len < 4 && (c & 0x80) != 0);
	return value;
}

/** put 1 byte */
static INLINE unsigned int mput1(unsigned int value, uint8_t* data)
{
	data[0] = value & 0xff;
	return data[0];
}

/** put 2 bytes (little-endian) */
static INLINE unsigned int mput2l(unsigned int value, uint8_t* data)
{
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	return data[1];
}

/** put 3 bytes (little-endian) */
static INLINE unsigned int mput3l(unsigned int value, uint8_t* data)
{
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = (value >> 16) & 0xff;
	return data[2];
}

/** put 4 bytes (little-endian) */
static INLINE unsigned int mput4l(unsigned int value, uint8_t* data)
{
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = (value >> 16) & 0xff;
	data[3] = (value >> 24) & 0xff;
	return data[3];
}

/** put variable-length integer (little-endian) */
static INLINE int mputvl(unsigned int value, uint8_t* data)
{
	int i;
	int len = varintlen(value);
	for (i = 0; i <len; i++)
	{
		data[i] = ((value >> (7 * i)) & 0x7F) | (i < len - 1) ? 0x80 : 0;
	}
	return data[len - 1];
}

/** put 2 bytes (big-endian) */
static INLINE unsigned int mput2b(unsigned int value, uint8_t* data)
{
	data[1] = value & 0xff;
	data[0] = (value >> 8) & 0xff;
	return data[0];
}

/** put 3 bytes (big-endian) */
static INLINE unsigned int mput3b(unsigned int value, uint8_t* data)
{
	data[2] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[0] = (value >> 16) & 0xff;
	return data[0];
}

/** put 4 bytes (big-endian) */
static INLINE unsigned int mput4b(unsigned int value, uint8_t* data)
{
	data[3] = value & 0xff;
	data[2] = (value >> 8) & 0xff;
	data[1] = (value >> 16) & 0xff;
	data[0] = (value >> 24) & 0xff;
	return data[0];
}

/** put variable-length integer (big-endian) */
static INLINE int mputvb(unsigned int value, uint8_t* data)
{
	int i;
	int len = varintlen(value);
	for (i = 0; i <len; i++)
	{
		data[i] = ((value >> (7 * (len - i - 1))) & 0x7F) | (i < len - 1) ? 0x80 : 0;
	}
	return data[0];
}

/** get 1 byte from file */
static INLINE unsigned int fget1(FILE* stream)
{
	return fgetc(stream);
}

/** get 2 bytes from file (little-endian) */
static INLINE unsigned int fget2l(FILE* stream)
{
	int b1;
	int b2;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF))
	{
		return b1 | (b2 * 0x0100);
	}
	return EOF;
}

/** get 3 bytes from file (little-endian) */
static INLINE unsigned int fget3l(FILE* stream)
{
	int b1;
	int b2;
	int b3;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	b3 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
	{
		return b1 | (b2 * 0x0100) | (b3 * 0x010000);
	}
	return EOF;
}

/** get 4 bytes from file (little-endian) */
static INLINE unsigned int fget4l(FILE* stream)
{
	int b1;
	int b2;
	int b3;
	int b4;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	b3 = fgetc(stream);
	b4 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
	{
		return b1 | (b2 * 0x0100) | (b3 * 0x010000) | (b4 * 0x01000000);
	}
	return EOF;
}

/** get variable-length integer to file (little-endian) */
static INLINE unsigned int fgetvl(FILE* stream)
{
	unsigned int value = 0;
	int len = 0;
	int c;
	do
	{
		c = fgetc(stream);
		if (c == EOF)
		{
			return EOF;
		}
		value |= (c & 0x7F) << (7 * len);
		len++;
	} while (len < 4 && (c & 0x80) != 0);
	return value;
}

/** get 2 bytes from file (big-endian) */
static INLINE unsigned int fget2b(FILE* stream)
{
	int b1;
	int b2;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF))
	{
		return b2 | (b1 * 0x0100);
	}
	return EOF;
}

/** get 3 bytes from file (big-endian) */
static INLINE unsigned int fget3b(FILE* stream)
{
	int b1;
	int b2;
	int b3;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	b3 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF) && (b3 != EOF))
	{
		return b3 | (b2 * 0x0100) | (b1 * 0x010000);
	}
	return EOF;
}

/** get 4 bytes from file (big-endian) */
static INLINE unsigned int fget4b(FILE* stream)
{
	int b1;
	int b2;
	int b3;
	int b4;

	b1 = fgetc(stream);
	b2 = fgetc(stream);
	b3 = fgetc(stream);
	b4 = fgetc(stream);
	if((b1 != EOF) && (b2 != EOF) && (b3 != EOF) && (b4 != EOF))
	{
		return b4 | (b3 * 0x0100) | (b2 * 0x010000) | (b1 * 0x01000000);
	}
	return EOF;
}

/** get variable-length integer to file (big-endian) */
static INLINE unsigned int fgetvb(FILE* stream)
{
	unsigned int value = 0;
	int len = 0;
	int c;
	do
	{
		c = fgetc(stream);
		if (c == EOF)
		{
			return EOF;
		}
		value = (value << 7) | (c & 0x7F);
		len++;
	} while (len < 4 && (c & 0x80) != 0);
	return value;
}

/** put 1 byte to file */
static INLINE unsigned int fput1(unsigned int value, FILE* stream)
{
	return fputc(value & 0xff, stream);
}

/** put 2 bytes to file (little-endian) */
static INLINE unsigned int fput2l(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc(value & 0xff, stream);
	if(result != EOF)
	{
		result = fputc((value >> 8) & 0xff, stream);
	}
	return result;
}

/** put 3 bytes to file (little-endian) */
static INLINE unsigned int fput3l(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc(value & 0xff, stream);
	if(result != EOF)
	{
		result = fputc((value >> 8) & 0xff, stream);
		if(result != EOF)
		{
			result = fputc((value >> 16) & 0xff, stream);
		}
	}
	return result;
}

/** put 4 bytes to file (little-endian) */
static INLINE unsigned int fput4l(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc(value & 0xff, stream);
	if(result != EOF)
	{
		result = fputc((value >> 8) & 0xff, stream);
		if(result != EOF)
		{
			result = fputc((value >> 16) & 0xff, stream);
			if(result != EOF)
			{
				result = fputc((value >> 24) & 0xff, stream);
			}
		}
	}
	return result;
}

/** put variable-length integer to file (little-endian) */
static INLINE unsigned int fputvl(unsigned int value, FILE* stream)
{
	int i;
	int len = varintlen(value);
	unsigned int result;
	for (i = 0; i < len; i++)
	{
		result = fputc(((value >> (7 * i)) & 0x7F) | ((i < len - 1) ? 0x80 : 0), stream);
		if (result == EOF)
		{
			return EOF;
		}
	}
	return result;
}

/** put 2 bytes to file (big-endian) */
static INLINE unsigned int fput2b(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc((value >> 8) & 0xff, stream);
	if(result != EOF)
	{
		result = fputc(value & 0xff, stream);
	}
	return result;
}

/** put 3 bytes to file (big-endian) */
static INLINE unsigned int fput3b(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc((value >> 16) & 0xff, stream);
	if(result != EOF)
	{
		result = fputc((value >> 8) & 0xff, stream);
		if(result != EOF)
		{
			result = fputc(value & 0xff, stream);
		}
	}
	return result;
}

/** put 4 bytes to file (big-endian) */
static INLINE unsigned int fput4b(unsigned int value, FILE* stream)
{
	unsigned int result;

	result = fputc((value >> 24) & 0xff, stream);
	if(result != EOF)
	{
		result = fputc((value >> 16) & 0xff, stream);
		if(result != EOF)
		{
			result = fputc((value >> 8) & 0xff, stream);
			if(result != EOF)
			{
				result = fputc(value & 0xff, stream);
			}
		}
	}
	return result;
}

/** put variable-length integer to file (big-endian) */
static INLINE unsigned int fputvb(unsigned int value, FILE* stream)
{
	int i;
	int len = varintlen(value);
	unsigned int result;
	for (i = 0; i < len; i++)
	{
		result = fputc(((value >> (7 * (len - i - 1))) & 0x7F) | ((i < len - 1) ? 0x80 : 0), stream);
		if (result == EOF)
		{
			return EOF;
		}
	}
	return result;
}

#endif /* !CBYTEIO_H_INCLUDED */
