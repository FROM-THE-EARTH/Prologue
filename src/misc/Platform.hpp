#pragma once

// Macros to check which platform compiling on
#if defined(_WIN32) || defined(WIN32)
#define PLATFORM_WINDOWS 1
#else
#define PLATFORM_WINDOWS 0
#endif

#define PLATFORM_MACOS __APPLE__

// popen / pclose definition
#if PLATFORM_WINDOWS
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif
