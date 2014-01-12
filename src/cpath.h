/**
 * Inline file/directory path routines for C.
 */

#ifndef CPATH_H_INCLUDED
#define CPATH_H_INCLUDED

#include <string.h>

#ifdef _WIN32
#pragma comment(lib, "shlwapi")
#include <shlwapi.h>

#include <sys/stat.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#endif

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

static INLINE char *path_findbase(char *path)
{
#ifdef _WIN32
	return PathFindFileNameA(path);
#else
	char *pslash;

	if (path == NULL)
	{
		return NULL;
	}

	pslash = strrchr(path, '/');
	if (pslash != NULL)
	{
		return pslash + 1;
	}
	else
	{
		return path;
	}
#endif
}

static INLINE char *path_findext(char *path)
{
#ifdef _WIN32
	return PathFindExtensionA(path);
#else
	char *pdot;
	char *pslash;

	if (path == NULL)
	{
		return NULL;
	}

	pdot = strrchr(path, '.');
	pslash = strrchr(path, '/');
	if (pdot != NULL && (pslash == NULL || pdot > pslash))
	{
		return pdot;
	}
	else
	{
		return NULL;
	}
#endif
}

static INLINE void path_basename(char *path)
{
#ifdef _WIN32
	PathStripPathA(path);
#else
	basename(path);
#endif
}

static INLINE void path_dirname(char *path)
{
#ifdef _WIN32
	PathRemoveFileSpecA(path);
#else
	dirname(path);
#endif
}

static INLINE void path_stripext(char *path)
{
#ifdef _WIN32
	PathRemoveExtensionA(path);
#else
	char *pdot = path_findext(path);
	if (pdot != NULL)
	{
		*pdot = '\0';
	}
#endif
}

static INLINE bool isdir(const char *path)
{
	struct stat st;
	if (stat(path, &st) == 0)
	{
		if ((st.st_mode & S_IFDIR) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}

#endif /* !CPATH_H_INCLUDED */
