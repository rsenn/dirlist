#include <limits.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LINUX_LIMITS_H
#include <linux/limits.h>
#endif

#include "str.h"
#include "path.h"
#include "stralloc.h"

#ifndef PATH_MAX
#include "windoze.h"
#if WINDOWS_NATIVE
#include <windows.h>
#endif
#ifdef MAX_PATH 
#define PATH_MAX MAX_PATH
#endif
#endif
