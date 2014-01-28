
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#define PATH_MAX	_MAX_PATH
#endif

#define BUF_SIZE	16384

void printUsage(const char * cmd)
{
	printf("Usage: %s (options) [input file] [keyword-start] [keyword-end]\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");
	printf("-o filename\n");
	printf("  : Specify output filename\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	int argi;
	int argnum;

	char in_path[PATH_MAX];
	char out_path[PATH_MAX];

	FILE * in_fp = NULL;
	FILE * out_fp = NULL;

	unsigned char * buf = NULL;

	char *key_start = NULL;
	char *key_end = NULL;
	size_t len_start = 0;
	size_t len_end = 0;
	size_t len_out = 0;
	off_t off_start = -1;
	off_t off_end = -1;
	off_t off_base;
	off_t off_inbuf;

	bool result = false;

	strcpy(out_path, "a.out");

	argi = 1;
	while (argi < argc && argv[argi][0] == '-')
	{
		if (strcmp(argv[argi], "--help") == 0)
		{
			printUsage(argv[0]);
			goto finish;
		}
		else if (strcmp(argv[argi], "-o") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			strcpy(out_path, argv[argi + 1]);
			argi++;
		}
		else
		{
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			goto finish;
		}
		argi++;
	}
	argnum = argc - argi;

	if (argnum == 0)
	{
		printUsage(argv[0]);
		goto finish;
	}
	if (argnum != 3)
	{
		fprintf(stderr, "Error: Too few/many arguments\n");
		fprintf(stderr, "\n");
		printUsage(argv[0]);
		return 1;
	}

	strcpy(in_path, argv[argi]);
	key_start = argv[argi + 1];
	key_end = argv[argi + 2];
	len_start = strlen(key_start);
	len_end = strlen(key_end);

	if (len_start > BUF_SIZE || len_end > BUF_SIZE)
	{
		fprintf(stderr, "Error: Keyword too long\n");
		goto finish;
	}

	in_fp = fopen(in_path, "rb");
	if (in_fp == NULL)
	{
		fprintf(stderr, "Error: %s - Could not open\n", in_path);
		goto finish;
	}

	buf = (unsigned char *) malloc(BUF_SIZE * 2);
	if (buf == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}
	memset(buf, 0, BUF_SIZE * 2);

	off_base = 0;
	off_inbuf = 0;
	fread(buf, 1, BUF_SIZE * 2, in_fp);
	for (;;)
	{
		if (off_start < 0)
		{
			if (memcmp(&buf[off_inbuf], key_start, len_start) == 0)
			{
				off_start = off_base + off_inbuf;
				off_inbuf += len_start;
			}
			else
			{
				off_inbuf++;
			}
		}
		else
		{
			if (memcmp(&buf[off_inbuf], key_end, len_end) == 0)
			{
				off_end = off_base + off_inbuf;
				break;
			}
			else
			{
				off_inbuf++;
			}
		}

		if (off_inbuf >= BUF_SIZE)
		{
			off_base += BUF_SIZE;
			off_inbuf -= BUF_SIZE;
			memcpy(&buf[0], &buf[BUF_SIZE], BUF_SIZE);
			memset(&buf[BUF_SIZE], 0, BUF_SIZE);
			if (fread(&buf[BUF_SIZE], 1, BUF_SIZE, in_fp) == 0)
			{
				break;
			}
		}
	}

	if (off_start < 0)
	{
		fprintf(stderr, "Error: Start word \"%s\" could not be found\n", key_start);
		goto finish;
	}

	if (off_end < 0)
	{
		fprintf(stderr, "Error: End word \"%s\" could not be found\n", key_end);
		goto finish;
	}

	out_fp = fopen(out_path, "wb");
	if (out_fp == NULL)
	{
		fprintf(stderr, "Error: %s - Could not open\n", out_path);
		goto finish;
	}

	len_out = off_end - off_start - len_start;
	if (len_out > 0)
	{
		free(buf);

		buf = (unsigned char *) malloc(len_out);
		if (buf == NULL)
		{
			fprintf(stderr, "Error: Memory allocation error\n");
			goto finish;
		}

		if (fseek(in_fp, off_start + len_start, SEEK_SET) != 0)
		{
			fprintf(stderr, "Error: File seek error\n");
			goto finish;
		}

		if (fread(buf, 1, len_out, in_fp) != len_out)
		{
			fprintf(stderr, "Error: File read error\n");
			goto finish;
		}

		if (fwrite(buf, 1, len_out, out_fp) != len_out)
		{
			fprintf(stderr, "Error: File write error\n");
			goto finish;
		}
	}

	result = true;

finish:
	if (out_fp != NULL)
	{
		fclose(out_fp);
	}
	if (buf != NULL)
	{
		free(buf);
	}
	if (in_fp != NULL)
	{
		fclose(in_fp);
	}
	return result ? 0 : 1;
}
