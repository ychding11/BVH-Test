#ifndef POINTS_H 
#define POINTS_H

#include "mei.h"

#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <random>
#include <fstream>
#include <iomanip>

namespace mei
{

	template <typename T>
	inline bool isNaN(const T x) {
		return std::isnan(x);
	}
	template <>
	inline bool isNaN(const int x) {
		return false;
	}

	// Vector Declarations
	template <typename T>
	class Vector2 {
	public:
		// Vector2 Public Methods
		Vector2() { x = y = 0; }
		Vector2(T xx, T yy) : x(xx), y(yy) { DCHECK(!HasNaNs()); }
		bool HasNaNs() const { return isNaN(x) || isNaN(y); }
		explicit Vector2(const Point2<T> &p);
		//explicit Vector2(const Point3<T> &p);
#ifndef NDEBUG
		// The default versions of these are fine for release builds; for debug
		// we define them so that we can add the Assert checks.
		Vector2(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x = v.x;
			y = v.y;
		}
		Vector2<T> &operator=(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x = v.x;
			y = v.y;
			return *this;
		}
#endif  // !NDEBUG
		
		Vector2<T> operator+(const Vector2<T> &v) const {
			DCHECK(!v.HasNaNs());
			return Vector2(x + v.x, y + v.y);
		}

		Vector2<T> &operator+=(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x += v.x;
			y += v.y;
			return *this;
		}
		Vector2<T> operator-(const Vector2<T> &v) const {
			DCHECK(!v.HasNaNs());
			return Vector2(x - v.x, y - v.y);
		}

		Vector2<T> &operator-=(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x -= v.x;
			y -= v.y;
			return *this;
		}
		bool operator==(const Vector2<T> &v) const { return x == v.x && y == v.y; }
		bool operator!=(const Vector2<T> &v) const { return x != v.x || y != v.y; }
		template <typename U>
		Vector2<T> operator*(U f) const {
			return Vector2<T>(f * x, f * y);
		}

		template <typename U>
		Vector2<T> &operator*=(U f) {
			DCHECK(!isNaN(f));
			x *= f;
			y *= f;
			return *this;
		}
		template <typename U>
		Vector2<T> operator/(U f) const {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			return Vector2<T>(x * inv, y * inv);
		}

		template <typename U>
		Vector2<T> &operator/=(U f) {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			x *= inv;
			y *= inv;
			return *this;
		}
		Vector2<T> operator-() const { return Vector2<T>(-x, -y); }
		T operator[](int i) const {
			DCHECK(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}

		T &operator[](int i) {
			DCHECK(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}
		Float LengthSquared() const { return x * x + y * y; }
		Float Length() const { return std::sqrt(LengthSquared()); }

		std::string str() const
		{
			std::ostringstream ss;
			ss << std::setprecision(5) << "[" << x << ", " << y << "]";
			return ss.str();
		}

		// Vector2 Public Data
		T x, y;
	};

	// Point Declarations
	template <typename T>
	class Point2 {
	public:
		// Point2 Public Methods
		//explicit Point2(const Point3<T> &p) : x(p.x), y(p.y) { DCHECK(!HasNaNs()); }
		Point2() { x = y = 0; }
		Point2(T xx, T yy) : x(xx), y(yy) { DCHECK(!HasNaNs()); }

		template <typename U>
		explicit Point2(const Point2<U> &p) {
			x = (T)p.x;
			y = (T)p.y;
			DCHECK(!HasNaNs());
		}

		template <typename U>
		explicit Point2(const Vector2<U> &p) {
			x = (T)p.x;
			y = (T)p.y;
			DCHECK(!HasNaNs());
		}

		template <typename U>
		explicit operator Vector2<U>() const {
			return Vector2<U>(x, y);
		}

#ifndef NDEBUG
		Point2(const Point2<T> &p) {
			DCHECK(!p.HasNaNs());
			x = p.x;
			y = p.y;
		}

		Point2<T> &operator=(const Point2<T> &p) {
			DCHECK(!p.HasNaNs());
			x = p.x;
			y = p.y;
			return *this;
		}
#endif  // !NDEBUG

		Point2<T> operator+(const Vector2<T> &v) const {
			DCHECK(!v.HasNaNs());
			return Point2<T>(x + v.x, y + v.y);
		}

		Point2<T> &operator+=(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x += v.x;
			y += v.y;
			return *this;
		}
		Vector2<T> operator-(const Point2<T> &p) const {
			DCHECK(!p.HasNaNs());
			return Vector2<T>(x - p.x, y - p.y);
		}

		Point2<T> operator-(const Vector2<T> &v) const {
			DCHECK(!v.HasNaNs());
			return Point2<T>(x - v.x, y - v.y);
		}
		Point2<T> operator-() const { return Point2<T>(-x, -y); }
		Point2<T> &operator-=(const Vector2<T> &v) {
			DCHECK(!v.HasNaNs());
			x -= v.x;
			y -= v.y;
			return *this;
		}
		Point2<T> &operator+=(const Point2<T> &p) {
			DCHECK(!p.HasNaNs());
			x += p.x;
			y += p.y;
			return *this;
		}
		Point2<T> operator+(const Point2<T> &p) const {
			DCHECK(!p.HasNaNs());
			return Point2<T>(x + p.x, y + p.y);
		}
		template <typename U>
		Point2<T> operator*(U f) const {
			return Point2<T>(f * x, f * y);
		}
		template <typename U>
		Point2<T> &operator*=(U f) {
			x *= f;
			y *= f;
			return *this;
		}
		template <typename U>
		Point2<T> operator/(U f) const {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			return Point2<T>(inv * x, inv * y);
		}
		template <typename U>
		Point2<T> &operator/=(U f) {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			x *= inv;
			y *= inv;
			return *this;
		}
		T operator[](int i) const {
			DCHECK(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}

		T &operator[](int i) {
			DCHECK(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}
		bool operator==(const Point2<T> &p) const { return x == p.x && y == p.y; }
		bool operator!=(const Point2<T> &p) const { return x != p.x || y != p.y; }
		bool HasNaNs() const { return isNaN(x) || isNaN(y); }

		std::string str() const
		{
			std::ostringstream ss;
			ss << std::setprecision(5) << "[" << x << ", " << y << "]";
			return ss.str();
		}

		// Point2 Public Data
		T x, y;
	};

	template <typename T>
	Vector2<T>::Vector2(const Point2<T> &p)
		: x(p.x), y(p.y) {
		DCHECK(!HasNaNs());
	}
}
#endif
