
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#define PATH_MAX	_MAX_PATH
#else
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#endif

void printUsage(const char * cmd)
{
	printf("Usage: %s (options) [input file]\n", cmd);
	printf("\n");
	printf("### Options ###\n");
	printf("\n");
	printf("-o filename\n");
	printf("  : Specify output filename\n");
	printf("\n");
	printf("-n variable-name\n");
	printf("  : Specify output variable name\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	int argi;
	int argnum;

	char in_path[PATH_MAX];
	char out_path[PATH_MAX];
	char var_name[256];

	FILE * in_fp = NULL;
	FILE * out_fp = NULL;
	size_t in_len = 0;

	unsigned char * buf = NULL;

	struct stat st;

	bool result = false;

	strcpy(out_path, "");
	strcpy(var_name, "a");

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
		else if (strcmp(argv[argi], "-n") == 0 || strcmp(argv[argi], "--name") == 0)
		{
			if (argi + 1 >= argc)
			{
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				goto finish;
			}
			strcpy(var_name, argv[argi + 1]);
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
	if (argnum != 1)
	{
		fprintf(stderr, "Error: Too few/many arguments\n");
		fprintf(stderr, "\n");
		printUsage(argv[0]);
		return 1;
	}

	strcpy(in_path, argv[argi]);

	if (stat(in_path, &st) != 0)
	{
		fprintf(stderr, "Error: %s - Could not get file stat\n", in_path);
		goto finish;
	}
	in_len = st.st_size;

	in_fp = fopen(in_path, "rb");
	if (in_fp == NULL)
	{
		fprintf(stderr, "Error: %s - Could not open\n", in_path);
		goto finish;
	}

	buf = (unsigned char *) malloc(in_len + 1);
	if (buf == NULL)
	{
		fprintf(stderr, "Error: Memory allocation error\n");
		goto finish;
	}

	if (fread(buf, 1, in_len, in_fp) != in_len)
	{
		fprintf(stderr, "Error: File read error\n");
		goto finish;
	}

	fclose(in_fp);
	in_fp = NULL;

	if (out_path[0] != '\0')
	{
		out_fp = fopen(out_path, "w");
		if (out_fp == NULL)
		{
			fprintf(stderr, "Error: %s - Could not open\n", out_path);
			goto finish;
		}
	}
	else
	{
		out_fp = stdout;
	}

	fprintf(out_fp, "\n");
	fprintf(out_fp, "static unsigned char %s[%u] = {", var_name, in_len);
	for (size_t i = 0; i < in_len; i++)
	{
		if (i % 16 == 0)
		{
			fprintf(out_fp, "\n\t");
		}
		else
		{
			fprintf(out_fp, " ");
		}
		fprintf(out_fp, "0x%02X,", buf[i]);
	}
	fprintf(out_fp, "\n");
	fprintf(out_fp, "};\n");

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
