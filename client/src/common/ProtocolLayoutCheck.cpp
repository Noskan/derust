#include "core/pch.h"
#include "common/Basedef.h"

#if defined(_WIN32) && !defined(_WIN64)
static_assert(sizeof(MSG_STANDARD) == 12, "MSG_STANDARD deve ter 12 bytes em Win32.");
static_assert(sizeof(STRUCT_ITEM) == 8, "STRUCT_ITEM deve ter 8 bytes em Win32.");
static_assert(sizeof(STRUCT_SCORE) == 48, "STRUCT_SCORE deve ter 48 bytes em Win32.");
static_assert(sizeof(STRUCT_MOB) == 1024, "STRUCT_MOB deve ter 1024 bytes em Win32.");
#endif
