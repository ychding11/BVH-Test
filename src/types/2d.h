
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

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
	// Forward Declarations
    template <typename T>
    class Vector2;
    template <typename T>
    class Point2;


	template <typename T>
	class Vector2
    {
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
		
		Vector2<T> operator+(const Vector2<T> &v) const
        {
			DCHECK(!v.HasNaNs());
			return Vector2(x + v.x, y + v.y);
		}

		Vector2<T> &operator+=(const Vector2<T> &v)
        {
			DCHECK(!v.HasNaNs());
			x += v.x;
			y += v.y;
			return *this;
		}
		Vector2<T> operator-(const Vector2<T> &v) const
        {
			DCHECK(!v.HasNaNs());
			return Vector2(x - v.x, y - v.y);
		}

		Vector2<T> &operator-=(const Vector2<T> &v)
        {
			DCHECK(!v.HasNaNs());
			x -= v.x;
			y -= v.y;
			return *this;
		}
		bool operator==(const Vector2<T> &v) const { return x == v.x && y == v.y; }
		bool operator!=(const Vector2<T> &v) const { return x != v.x || y != v.y; }

		template <typename U>
		Vector2<T> operator*(U f) const
        {
			return Vector2<T>(f * x, f * y);
		}

		template <typename U>
		Vector2<T>& operator*=(U f)
        {
			DCHECK(!isNaN(f));
			x *= f;
			y *= f;
			return *this;
		}
		template <typename U>
		Vector2<T> operator/(U f) const
        {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			return Vector2<T>(x * inv, y * inv);
		}

		template <typename U>
		Vector2<T> &operator/=(U f)
        {
			CHECK_NE(f, 0);
			Float inv = (Float)1 / f;
			x *= inv;
			y *= inv;
			return *this;
		}
		Vector2<T> operator-() const { return Vector2<T>(-x, -y); }

		T operator[](int i) const
        {
			DCHECK(i >= 0 && i <= 1);
			if (i == 0) return x;
			return y;
		}

		T &operator[](int i)
        {
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
	class Point2
    {
	public:
		// Point2 Public Methods
		//explicit Point2(const Point3<T> &p) : x(p.x), y(p.y) { DCHECK(!HasNaNs()); }
		Point2() { x = y = 0; }
		Point2(T xx, T yy) : x(xx), y(yy) { DCHECK(!HasNaNs()); }

		template <typename U>
		explicit Point2(const Point2<U> &p)
        {
			x = (T)p.x;
			y = (T)p.y;
			DCHECK(!HasNaNs());
		}

		template <typename U>
		explicit Point2(const Vector2<U> &p)
        {
			x = (T)p.x;
			y = (T)p.y;
			DCHECK(!HasNaNs());
		}

		template <typename U>
		explicit operator Vector2<U>() const
        {
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
	Point2<T> Min(const Point2<T> &pa, const Point2<T> &pb)
    {
		return Point2<T>(std::min(pa.x, pb.x), std::min(pa.y, pb.y));
	}

	template <typename T>
	Point2<T> Max(const Point2<T> &pa, const Point2<T> &pb)
    {
		return Point2<T>(std::max(pa.x, pb.x), std::max(pa.y, pb.y));
	}

	template <typename T>
	Point2<T> Floor(const Point2<T> &p)
    {
		return Point2<T>(std::floor(p.x), std::floor(p.y));
	}

	template <typename T>
	Point2<T> Ceil(const Point2<T> &p)
    {
		return Point2<T>(std::ceil(p.x), std::ceil(p.y));
	}

    typedef Point2<Float> Point2f;
    typedef Point2<int> Point2i;

	template <typename T>
	Vector2<T>::Vector2(const Point2<T> &p)
		: x(p.x), y(p.y)
    {
		DCHECK(!HasNaNs());
	}

	typedef Vector2<Float> Vector2f;
	typedef Vector2<int> Vector2i;

	template <typename T>
	inline Float Distance(const Point2<T> &p1, const Point2<T> &p2)
    {
		return (p1 - p2).Length();
	}

	// Bounds Declarations
	template <typename T>
	class Bounds2
    {
	public:
		// Bounds2 Public Methods
		Bounds2()
        {
			T minNum = std::numeric_limits<T>::lowest();
			T maxNum = std::numeric_limits<T>::max();
			pMin = Point2<T>(maxNum, maxNum);
			pMax = Point2<T>(minNum, minNum);
		}
		explicit Bounds2(const Point2<T> &p) : pMin(p), pMax(p) {}
		Bounds2(const Point2<T> &p1, const Point2<T> &p2)
        {
			pMin = Point2<T>(std::min(p1.x, p2.x), std::min(p1.y, p2.y));
			pMax = Point2<T>(std::max(p1.x, p2.x), std::max(p1.y, p2.y));
		}
		template <typename U>
		explicit operator Bounds2<U>() const {
			return Bounds2<U>((Point2<U>)pMin, (Point2<U>)pMax);
		}

		Vector2<T> Diagonal() const { return pMax - pMin; }
		T Area() const
        {
			Vector2<T> d = pMax - pMin;
			return (d.x * d.y);
		}
		int MaximumExtent() const
        {
			Vector2<T> diag = Diagonal();
			if (diag.x > diag.y)
				return 0;
			else
				return 1;
		}
		inline const Point2<T> &operator[](int i) const {
			DCHECK(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}
		inline Point2<T> &operator[](int i) {
			DCHECK(i == 0 || i == 1);
			return (i == 0) ? pMin : pMax;
		}
		bool operator==(const Bounds2<T> &b) const {
			return b.pMin == pMin && b.pMax == pMax;
		}
		bool operator!=(const Bounds2<T> &b) const {
			return b.pMin != pMin || b.pMax != pMax;
		}
		Point2<T> Lerp(const Point2f &t) const
        {
			return Point2<T>(pbrt::Lerp(t.x, pMin.x, pMax.x),
				pbrt::Lerp(t.y, pMin.y, pMax.y));
		}
		Vector2<T> Offset(const Point2<T> &p) const
        {
			Vector2<T> o = p - pMin;
			if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
			if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
			return o;
		}
		void BoundingSphere(Point2<T> *c, Float *rad) const {
			*c = (pMin + pMax) / 2;
			*rad = Inside(*c, *this) ? Distance(*c, pMax) : 0;
		}
		Point2<T> Center() const
        {
			return Point2<T>((pMax.x+pMin.x)/2,(pMax.x+pMin.x)/2 );
		}

		std::string str() const
		{
			return pMin.str() + pMax.str();
		}

		// Bounds2 Public Data
		Point2<T> pMin, pMax;
	};

	template <typename T>
	bool InsideExclusive(const Point2<T> &pt, const Bounds2<T> &b)
    {
		return (pt.x >= b.pMin.x && pt.x < b.pMax.x && pt.y >= b.pMin.y && pt.y < b.pMax.y);
	}


    template <typename T>
    Bounds2<T> Intersect(const Bounds2<T> &b1, const Bounds2<T> &b2)
    {
        // Important: assign to pMin/pMax directly and don't run the Bounds2()
        // constructor, since it takes min/max of the points passed to it.  In
        // turn, that breaks returning an invalid bound for the case where we
        // intersect non-overlapping bounds (as we'd like to happen).
        Bounds2<T> ret;
        ret.pMin = Max(b1.pMin, b2.pMin);
        ret.pMax = Min(b1.pMax, b2.pMax);
        return ret;
    }


	typedef Bounds2<Float> Bounds2f;
	typedef Bounds2<int> Bounds2i;

    //< define how to visit all points in a 2D Bounds.
	class Bounds2iIterator : public std::forward_iterator_tag
    {
	public:
		Bounds2iIterator(const Bounds2i &b, const Point2i &pt)
			: p(pt), bounds(&b) {}
		Bounds2iIterator operator++() {
			advance();
			return *this;
		}
		Bounds2iIterator operator++(int) {
			Bounds2iIterator old = *this;
			advance();
			return old;
		}
		bool operator==(const Bounds2iIterator &bi) const {
			return p == bi.p && bounds == bi.bounds;
		}
		bool operator!=(const Bounds2iIterator &bi) const {
			return p != bi.p || bounds != bi.bounds;
		}

		Point2i operator*() const { return p; }

	private:
		void advance() {
			++p.x;
			if (p.x == bounds->pMax.x) {
				p.x = bounds->pMin.x;
				++p.y;
			}
		}
		Point2i p;
		const Bounds2i *bounds;
	};

	inline Bounds2iIterator begin(const Bounds2i &b)
    {
		return Bounds2iIterator(b, b.pMin);
	}

	inline Bounds2iIterator end(const Bounds2i &b)
    {
		// Normally, the ending point is at the minimum x value and one past
		// the last valid y value.
		Point2i pEnd(b.pMin.x, b.pMax.y);
		// However, if the bounds are degenerate, override the end point to
		// equal the start point so that any attempt to iterate over the bounds
		// exits out immediately.
		if (b.pMin.x >= b.pMax.x || b.pMin.y >= b.pMax.y)
			pEnd = b.pMin;
		return Bounds2iIterator(b, pEnd);
	}

}
#endif
