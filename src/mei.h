#ifndef MEI_H 
#define MEI_H

#include <type_traits>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <assert.h>
#include <string.h>
#include <glog/logging.h>

// Platform-specific definitions
#if defined(_WIN32) || defined(_WIN64)
#define IN_WINDOWS
#endif

#if defined(_MSC_VER)
	#define IN_MSVC
	#if _MSC_VER == 1800
	#define snprintf _snprintf
	#endif
#endif;

#include <stdint.h>

#if defined(IN_MSVC)
	#include <float.h>
	#include <intrin.h>
	#pragma warning(disable : 4305)  // double constant assigned to float
	#pragma warning(disable : 4244)  // int -> float conversion
	#pragma warning(disable : 4843)  // double -> float conversion
	#pragma warning(disable : 4267)  // size_t -> int
	#pragma warning(disable : 4838)  // another double -> int
#endif

namespace mei
{

#ifndef M_PI
#define M_PI  3.1415926
#endif
	typedef double Float;

	static const Float inf = 1e20;
	static const Float eps = 1e-6;

	template <typename T>
	class Vector2;
	template <typename T>
	class Point2;

}

#endif
