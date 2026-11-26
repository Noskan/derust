#include "common/Basedef.h"

#if defined(_WIN32) && !defined(_WIN64)
static_assert(sizeof(MSG_STANDARD) == 12, "MSG_STANDARD deve ter 12 bytes em Win32.");
static_assert(sizeof(STRUCT_ITEM) == 8, "STRUCT_ITEM deve ter 8 bytes em Win32.");
static_assert(sizeof(STRUCT_SCORE) == 48, "STRUCT_SCORE deve ter 48 bytes em Win32.");
static_assert(sizeof(STRUCT_MOB) == 816, "STRUCT_MOB deve ter 816 bytes em Win32.");
static_assert(sizeof(STRUCT_MOBEXTRA) == 536, "STRUCT_MOBEXTRA deve ter 536 bytes em Win32 (time_t de 64 bits).");
static_assert(sizeof(STRUCT_RANKING) == 52, "STRUCT_RANKING deve ter 52 bytes em Win32.");
static_assert(sizeof(STRUCT_ACCOUNTFILE) == 7800, "STRUCT_ACCOUNTFILE deve ter 7800 bytes em Win32.");
#endif
