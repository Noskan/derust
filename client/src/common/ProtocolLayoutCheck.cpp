#include "core/pch.h"
#include "common/Basedef.h"

#include <cstddef>

namespace
{
	template <std::size_t Actual, std::size_t Expected>
	struct CheckStructSize
	{
		static_assert(Actual == Expected, "Layout binario Win32 divergente para struct de protocolo do cliente.");
		static constexpr bool value = true;
	};
}

#if defined(_WIN32) && !defined(_WIN64)
static_assert(CheckStructSize<sizeof(MSG_STANDARD), 12>::value, "MSG_STANDARD");
static_assert(CheckStructSize<sizeof(STRUCT_ITEM), 8>::value, "STRUCT_ITEM");
static_assert(CheckStructSize<sizeof(STRUCT_SCORE), 48>::value, "STRUCT_SCORE");
static_assert(CheckStructSize<sizeof(STRUCT_MOB), 1024>::value, "STRUCT_MOB");
#endif
