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
#define ON_WINDOWS
#endif


#ifdef __GNUG__
#define ON_LINUX
#else
#endif  // __GNUG__

// Check OS & Platform
#if defined(_WIN32) || defined(_WIN64)
#define ON_WINDOWS
#if defined(__MINGW32__)
#define ON_MINGW
#elif defined(_MSC_VER)
#define ON_MSVC
#endif
#elif defined(__linux__)
#define ON_LINUX
#elif defined(__APPLE__)
#define ON_OSX
#elif defined(__OpenBSD__)
#define ON_OPENBSD
#elif defined(__FreeBSD__)
#define ON_FREEBSD
#endif


#if defined(_MSC_VER)
	#define ON_MSVC
	#if _MSC_VER == 1800
	#define snprintf _snprintf
	#endif
#endif;

#include <stdint.h>

#if defined(ON_MSVC)
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

//typedef float Float;
#define Infinity std::numeric_limits<Float>::infinity()

	typedef double Float;

	static const Float inf = 1e20;
	static const Float eps = 1e-6;

	template <typename T>
	class Vector2;
	template <typename T>
	class Point2;

}

#endif
