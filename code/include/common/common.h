#ifndef BP_COMMON_H
#define BP_COMMON_H

#if defined(_WIN32)
#define BP_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#else
#define BP_PLATFORM_LINUX
#endif

#include "bp_thread.h"
#include "bp_string.h"
#include "bp_mutex.h"
#include "bp_string.h"
#include "bp_fstring.h"
#include "bp_vector.h"
#include "bp_net.h"

#endif