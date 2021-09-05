// (c) orginal was made by Stefan Roettger, licensed under GPL 2+
// (c) Minor porting and edits were made by Thomas Clarke under GPL 3+
// see original code at https://sourceforge.net/projects/volren/

#include "ReadPVMvolume.h"

// Debug functions
void ReadPVMvolume::ERRORMSG()
{
	printf("bad thing happened");
}

void ReadPVMvolume::MEMERROR()
{
	printf("Ran out of memory");
}

void ReadPVMvolume::IOERROR()
{
	printf("Bad File Input");
}

// deinterleave a byte stream
void ReadPVMvolume::DDS_deinterleave(unsigned char *data, unsigned int bytes, unsigned int skip, unsigned int block, bool restore)
{
	unsigned int i, j, k;

	unsigned char *data2, *ptr;

	if (skip <= 1) return;

	if (block == 0)
	{
		if ((data2 = (unsigned char *)malloc(bytes)) == NULL) MEMERROR();

		if (!restore)
			for (ptr = data2, i = 0; i < skip; i++)
				for (j = i; j < bytes; j += skip) *ptr++ = data[j];
		else
			for (ptr = data, i = 0; i < skip; i++)
				for (j = i; j < bytes; j += skip) data2[j] = *ptr++;

		memcpy(data, data2, bytes);
	}
	else
	{
		if ((data2 = (unsigned char *)malloc((bytes < skip*block) ? bytes : skip * block)) == NULL) MEMERROR();

		if (!restore)
		{
			for (k = 0; k < bytes / skip / block; k++)
			{
				for (ptr = data2, i = 0; i < skip; i++)
					for (j = i; j < skip*block; j += skip) *ptr++ = data[k*skip*block + j];

				memcpy(data + k * skip*block, data2, skip*block);
			}

			for (ptr = data2, i = 0; i < skip; i++)
				for (j = i; j < bytes - k * skip*block; j += skip) *ptr++ = data[k*skip*block + j];

			memcpy(data + k * skip*block, data2, bytes - k * skip*block);
		}
		else
		{
			for (k = 0; k < bytes / skip / block; k++)
			{
				for (ptr = data + k * skip*block, i = 0; i < skip; i++)
					for (j = i; j < skip*block; j += skip) data2[j] = *ptr++;

				memcpy(data + k * skip*block, data2, skip*block);
			}

			for (ptr = data + k * skip*block, i = 0; i < skip; i++)
				for (j = i; j < bytes - k * skip*block; j += skip) data2[j] = *ptr++;

			memcpy(data + k * skip*block, data2, bytes - k * skip*block);
		}
	}

	free(data2);
}

// read from a RAW file
unsigned char * ReadPVMvolume::readRAWfiled(FILE *file, unsigned int *bytes)
{
	unsigned char *data;
	unsigned int cnt, blkcnt;

	data = NULL;
	cnt = 0;

	do
	{
		if (data == NULL)
		{
			if ((data = (unsigned char *)malloc(DDS_BLOCKSIZE)) == NULL) MEMERROR();
		}
		else
			if ((data = (unsigned char *)realloc(data, cnt + DDS_BLOCKSIZE)) == NULL) MEMERROR();

		blkcnt = fread(&data[cnt], 1, DDS_BLOCKSIZE, file);
		cnt += blkcnt;
	} while (blkcnt == DDS_BLOCKSIZE);

	if (cnt == 0)
	{
		free(data);
		return(NULL);
	}

	if ((data = (unsigned char *)realloc(data, cnt)) == NULL) MEMERROR();

	*bytes = cnt;

	return(data);
}

// decode a Differential Data Stream
void ReadPVMvolume::DDS_decode(unsigned char *chunk, unsigned int size,
	unsigned char **data, unsigned int *bytes,
	unsigned int block)
{
	unsigned int skip, strip;

	unsigned char *ptr1, *ptr2;

	unsigned int cnt, cnt1, cnt2;
	int bits, act;

	DDS_initbuffer();

	DDS_clearbits();
	DDS_loadbits(chunk, size);

	skip = DDS_readbits(2) + 1;
	strip = DDS_readbits(16) + 1;

	ptr1 = ptr2 = NULL;
	cnt = act = 0;

	while ((cnt1 = DDS_readbits(DDS_RL)) != 0)
	{
		bits = DDS_decode(DDS_readbits(3));

		for (cnt2 = 0; cnt2 < cnt1; cnt2++)
		{
			if (strip == 1 || cnt <= strip) act += DDS_readbits(bits) - (1 << bits) / 2;
			else act += *(ptr2 - strip) - *(ptr2 - strip - 1) + DDS_readbits(bits) - (1 << bits) / 2;

			while (act < 0) act += 256;
			while (act > 255) act -= 256;

			if ((cnt&(DDS_BLOCKSIZE - 1)) == 0)
				if (ptr1 == NULL)
				{
					if ((ptr1 = (unsigned char *)malloc(DDS_BLOCKSIZE)) == NULL) MEMERROR();
					ptr2 = ptr1;
				}
				else
				{
					if ((ptr1 = (unsigned char *)realloc(ptr1, cnt + DDS_BLOCKSIZE)) == NULL) MEMERROR();
					ptr2 = &ptr1[cnt];
				}

			*ptr2++ = act;
			cnt++;
		}
	}

	if (ptr1 != NULL)
		if ((ptr1 = (unsigned char *)realloc(ptr1, cnt)) == NULL) MEMERROR();

	DDS_interleave(ptr1, cnt, skip, block);

	*data = ptr1;
	*bytes = cnt;
}

unsigned char * ReadPVMvolume::readPVMvolume(const char * filename, unsigned int * width, unsigned int * height, unsigned int * depth, unsigned int * components, float * scalex, float * scaley, float * scalez, unsigned char ** description, unsigned char ** courtesy, unsigned char ** parameter, unsigned char ** comment)
{
	unsigned char *data, *ptr;
	unsigned int bytes, numc;

	int version = 1;

	unsigned char *volume;

	float sx = 1.0f, sy = 1.0f, sz = 1.0f;

	unsigned int len1 = 0, len2 = 0, len3 = 0, len4 = 0;

	if ((data = readDDSfile(filename, &bytes)) == NULL)
		if ((data = readRAWfile(filename, &bytes)) == NULL) return(NULL);

	if (bytes < 5) return(NULL);

	if ((data = (unsigned char *)realloc(data, bytes + 1)) == NULL) MEMERROR();
	data[bytes] = '\0';

	if (strncmp((char *)data, "PVM\n", 4) != 0)
	{
		if (strncmp((char *)data, "PVM2\n", 5) == 0) version = 2;
		else if (strncmp((char *)data, "PVM3\n", 5) == 0) version = 3;
		else return(NULL);

		ptr = &data[5];
		if (sscanf_s((char *)ptr, "%d %d %d\n%g %g %g\n", width, height, depth, &sx, &sy, &sz) != 6) ERRORMSG();
		if (*width < 1 || *height < 1 || *depth < 1 || sx <= 0.0f || sy <= 0.0f || sz <= 0.0f) ERRORMSG();
		ptr = (unsigned char *)strchr((char *)ptr, '\n') + 1;
	}
	else
	{
		ptr = &data[4];
		while (*ptr == '#')
			while (*ptr++ != '\n');

		if (sscanf_s((char *)ptr, "%d %d %d\n", width, height, depth) != 3) ERRORMSG();
		if (*width < 1 || *height < 1 || *depth < 1) ERRORMSG();
	}

	if (scalex != NULL && scaley != NULL && scalez != NULL)
	{
		*scalex = sx;
		*scaley = sy;
		*scalez = sz;
	}

	ptr = (unsigned char *)strchr((char *)ptr, '\n') + 1;
	if (sscanf_s((char *)ptr, "%d\n", &numc) != 1) ERRORMSG();
	if (numc < 1) ERRORMSG();

	if (components != NULL) *components = numc;
	else if (numc != 1) ERRORMSG();

	ptr = (unsigned char *)strchr((char *)ptr, '\n') + 1;
	if (version == 3) len1 = strlen((char *)(ptr + (*width)*(*height)*(*depth)*numc)) + 1;
	if (version == 3) len2 = strlen((char *)(ptr + (*width)*(*height)*(*depth)*numc + len1)) + 1;
	if (version == 3) len3 = strlen((char *)(ptr + (*width)*(*height)*(*depth)*numc + len1 + len2)) + 1;
	if (version == 3) len4 = strlen((char *)(ptr + (*width)*(*height)*(*depth)*numc + len1 + len2 + len3)) + 1;
	if ((volume = (unsigned char *)malloc((*width)*(*height)*(*depth)*numc + len1 + len2 + len3 + len4)) == NULL) MEMERROR();
	if (data + bytes != ptr + (*width)*(*height)*(*depth)*numc + len1 + len2 + len3 + len4) ERRORMSG();

	memcpy(volume, ptr, (*width)*(*height)*(*depth)*numc + len1 + len2 + len3 + len4);
	free(data);

	if (description != NULL)
		if (len1 > 1) *description = volume + (*width)*(*height)*(*depth)*numc;
		else *description = NULL;

	if (courtesy != NULL)
		if (len2 > 1) *courtesy = volume + (*width)*(*height)*(*depth)*numc + len1;
		else *courtesy = NULL;

	if (parameter != NULL)
		if (len3 > 1) *parameter = volume + (*width)*(*height)*(*depth)*numc + len1 + len2;
		else *parameter = NULL;

	if (comment != NULL)
		if (len4 > 1) *comment = volume + (*width)*(*height)*(*depth)*numc + len1 + len2 + len3;
		else *comment = NULL;

	return(volume);
}

unsigned char * ReadPVMvolume::readRAWfile(const char * filename, unsigned int * bytes)
{
	FILE *file;

	unsigned char *data;

	if ((file = fopen(filename, "rb")) == NULL) return(NULL);

	data = readRAWfiled(file, bytes);

	fclose(file);

	return(data);
}


unsigned char * ReadPVMvolume::readDDSfile(const char *filename, unsigned int *bytes)
{
	int version = 1;

	FILE *file;

	int cnt;

	unsigned char *chunk, *data;
	unsigned int size;

	if ((file = fopen(filename, "rb")) == NULL) return(NULL);

	for (cnt = 0; DDS_ID[cnt] != '\0'; cnt++) {
		if (fgetc(file) != DDS_ID[cnt])
		{
			fclose(file);
			version = 0;
			break;
		}
	}
		

	if (version == 0)
	{
		if ((file = fopen(filename, "rb")) == NULL) return(NULL);

		for (cnt = 0; DDS_ID2[cnt] != '\0'; cnt++)
			if (fgetc(file) != DDS_ID2[cnt])
			{
				fclose(file);
				return(NULL);
			}

		version = 2;
	}

	if ((chunk = readRAWfiled(file, &size)) == NULL) IOERROR();

	fclose(file);

	DDS_decode(chunk, size, &data, bytes, version == 1 ? 0 : DDS_INTERLEAVE);

	free(chunk);

	return(data);
}

// write a RAW file
void ReadPVMvolume::writeRAWfile(const char *filename, unsigned char *data, unsigned int bytes, bool nofree)
{
	FILE *file;

	if (bytes < 1) ERRORMSG();

	if ((file = fopen(filename, "wb")) == NULL) IOERROR();
	if (fwrite(data, 1, bytes, file) != bytes) IOERROR();

	fclose(file);

	if (!nofree) free(data);
}
