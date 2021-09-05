// (c) orginal was made by Stefan Roettger, licensed under GPL 2+
// (c) Minor porting and edits were made by Thomas Clarke under GPL 2+
// see original code at https://sourceforge.net/projects/volren/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <iostream>

// Definition stuff for the encription
#define DDS_MAXSTR (256)
#define DDS_INTERLEAVE (1<<24)
#define DDS_BLOCKSIZE (1<<20)
#define DDS_RL (7)
#define DDS_ISINTEL (*((unsigned char *)(&DDS_INTEL)+1)==0)


class ReadPVMvolume
{
public:

	void DDS_decode(unsigned char *chunk, unsigned int size,
		unsigned char **data, unsigned int *bytes,
		unsigned int block = 0);


	unsigned char *readPVMvolume(const char *filename,
		unsigned int *width, unsigned int *height, unsigned int *depth, unsigned int *components = NULL,
		float *scalex = NULL, float *scaley = NULL, float *scalez = NULL,
		unsigned char **description = NULL,
		unsigned char **courtesy = NULL,
		unsigned char **parameter = NULL,
		unsigned char **comment = NULL);

	unsigned char *readDDSfile(const char *filename, unsigned int *bytes);

	unsigned char *readRAWfiled(FILE *file, unsigned int *bytes);

	unsigned char *readRAWfile(const char *filename, unsigned int *bytes);

	void writeRAWfile(const char *filename, unsigned char *data, unsigned int bytes, bool nofree);

private:
	// Hard set values for reading encription
	std::string DDS_ID = "DDS v3d\n";
	std::string DDS_ID2 = "DDS v3e\n";

	unsigned char *DDS_cache;
	unsigned int DDS_cachepos, DDS_cachesize;

	unsigned int DDS_buffer;
	unsigned int DDS_bufsize;

	unsigned short int DDS_INTEL = 1;

	// Erorr messages
	static void ERRORMSG();
	static void MEMERROR();
	static void IOERROR();

	// things that don't need to be touched by the outside
	void DDS_deinterleave(unsigned char *data, unsigned int bytes, unsigned int skip, unsigned int block = 0, bool restore = false);

	// helper functions for DDS:

	inline unsigned int DDS_shiftl(const unsigned int value, const unsigned int bits)
	{
		return((bits >= 32) ? 0 : value << bits);
	}

	inline unsigned int DDS_shiftr(const unsigned int value, const unsigned int bits)
	{
		return((bits >= 32) ? 0 : value >> bits);
	}

	inline void DDS_swapuint(unsigned int *x)
	{
		unsigned int tmp = *x;

		*x = ((tmp & 0xff) << 24) |
			((tmp & 0xff00) << 8) |
			((tmp & 0xff0000) >> 8) |
			((tmp & 0xff000000) >> 24);
	}

	void DDS_initbuffer()
	{
		DDS_buffer = 0;
		DDS_bufsize = 0;
	}

	inline void DDS_clearbits()
	{
		DDS_cache = NULL;
		DDS_cachepos = 0;
		DDS_cachesize = 0;
	}

	inline void DDS_writebits(unsigned int value, unsigned int bits)
	{
		value &= DDS_shiftl(1, bits) - 1;

		if (DDS_bufsize + bits < 32)
		{
			DDS_buffer = DDS_shiftl(DDS_buffer, bits) | value;
			DDS_bufsize += bits;
		}
		else
		{
			DDS_buffer = DDS_shiftl(DDS_buffer, 32 - DDS_bufsize);
			DDS_bufsize -= 32 - bits;
			DDS_buffer |= DDS_shiftr(value, DDS_bufsize);

			if (DDS_cachepos + 4 > DDS_cachesize)
				if (DDS_cache == NULL)
				{
					if ((DDS_cache = (unsigned char *)malloc(DDS_BLOCKSIZE)) == NULL) MEMERROR();
					DDS_cachesize = DDS_BLOCKSIZE;
				}
				else
				{
					if ((DDS_cache = (unsigned char *)realloc(DDS_cache, DDS_cachesize + DDS_BLOCKSIZE)) == NULL) MEMERROR();
					DDS_cachesize += DDS_BLOCKSIZE;
				}

			if (DDS_ISINTEL) DDS_swapuint(&DDS_buffer);
			*((unsigned int *)&DDS_cache[DDS_cachepos]) = DDS_buffer;
			DDS_cachepos += 4;

			DDS_buffer = value & (DDS_shiftl(1, DDS_bufsize) - 1);
		}
	}

	inline void DDS_flushbits()
	{
		unsigned int bufsize;

		bufsize = DDS_bufsize;

		if (bufsize > 0)
		{
			DDS_writebits(0, 32 - bufsize);
			DDS_cachepos -= (32 - bufsize) / 8;
		}
	}

	inline void DDS_savebits(unsigned char **data, unsigned int *size)
	{
		*data = DDS_cache;
		*size = DDS_cachepos;
	}

	inline void DDS_loadbits(unsigned char *data, unsigned int size)
	{
		DDS_cache = data;
		DDS_cachesize = size;

		if ((DDS_cache = (unsigned char *)realloc(DDS_cache, DDS_cachesize + 4)) == NULL) MEMERROR();
		*((unsigned int *)&DDS_cache[DDS_cachesize]) = 0;

		DDS_cachesize = 4 * ((DDS_cachesize + 3) / 4);
		if ((DDS_cache = (unsigned char *)realloc(DDS_cache, DDS_cachesize)) == NULL) MEMERROR();
	}

	inline unsigned int DDS_readbits(unsigned int bits)
	{
		unsigned int value;

		if (bits < DDS_bufsize)
		{
			DDS_bufsize -= bits;
			value = DDS_shiftr(DDS_buffer, DDS_bufsize);
		}
		else
		{
			value = DDS_shiftl(DDS_buffer, bits - DDS_bufsize);

			if (DDS_cachepos >= DDS_cachesize) DDS_buffer = 0;
			else
			{
				DDS_buffer = *((unsigned int *)&DDS_cache[DDS_cachepos]);
				if (DDS_ISINTEL) DDS_swapuint(&DDS_buffer);
				DDS_cachepos += 4;
			}

			DDS_bufsize += 32 - bits;
			value |= DDS_shiftr(DDS_buffer, DDS_bufsize);
		}

		DDS_buffer &= DDS_shiftl(1, DDS_bufsize) - 1;

		return(value);
	}

	inline int DDS_code(int bits)
	{
		return(bits > 1 ? bits - 1 : bits);
	}

	inline int DDS_decode(int bits)
	{
		return(bits >= 1 ? bits + 1 : bits);
	}

	// interleave a byte stream
	void DDS_interleave(unsigned char *data, unsigned int bytes, unsigned int skip, unsigned int block = 0)
	{
		DDS_deinterleave(data, bytes, skip, block, true);
	}
};

