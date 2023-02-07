#pragma once

#include "mg/common/Definitions.h"

#define F_DECLARE_CLASS(NAMESPACE_1, NAMESPACE_2, CLASS_TYPE) \
	namespace NAMESPACE_1 { namespace NAMESPACE_2 { class CLASS_TYPE; } }

#define F_DECLARE_CLASS_3(NAMESPACE_1, NAMESPACE_2, NAMESPACE_3, CLASS_TYPE) \
	namespace NAMESPACE_1 { namespace NAMESPACE_2 { namespace NAMESPACE_3 { class CLASS_TYPE; } } }

#define F_DECLARE_STRUCT(NAMESPACE_1, NAMESPACE_2, STRUCT_TYPE) \
	namespace NAMESPACE_1 { namespace NAMESPACE_2 { struct STRUCT_TYPE; } }

#define F_DECLARE_STRUCT_3(NAMESPACE_1, NAMESPACE_2, NAMESPACE_3, STRUCT_TYPE) \
	namespace NAMESPACE_1 { namespace NAMESPACE_2 { namespace NAMESPACE_3 { struct STRUCT_TYPE; } } }

#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <new>

#if IS_PLATFORM_WIN
#include <Windows.h>
#endif

using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using float32 = float;
using float64 = double;

namespace mg {
namespace common {

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using float32 = float;
	using float64 = double;

}
}
