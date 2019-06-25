#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>
#include <iomanip>

namespace smallpt
{

#ifndef M_PI
#define M_PI  3.1415926
#endif
    typedef double Float;

    static const Float inf = 1e20;
    static const Float eps = 1e-6;

    struct Vector3
    {
        Float x, y, z;

        Vector3(Float x = 0, Float y = 0, Float z = 0) : x(x), y(y), z(z) { }
        Vector3(const Vector3 &b) : x(b.x), y(b.y), z(b.z) { }

        Vector3 operator+(const Vector3& b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
        Vector3 operator-(const Vector3& b) const { return Vector3(x - b.x, y - b.y, z - b.z); }

        Vector3 operator*(Float b) const { return Vector3(x*b, y*b, z*b); }
        Vector3 operator/(Float b) const { b = 1.f / b; return Vector3(x*b, y*b, z*b); }

        //Vector3 operator/(const Vector3 &b) const { return Vector3(x / b.x, y / b.y, z / b.z); }

        Vector3 operator+=(const Vector3& b) { return *this = Vector3(x + b.x, y + b.y, z + b.z); }
        Vector3 operator*=(Float b) { return *this = Vector3(x*b, y*b, z*b); }

        // Component-wise multiply and divide
        Vector3 cmult(const Vector3& b) const { return Vector3(x*b.x, y*b.y, z*b.z); }
        Vector3 cdiv(const Vector3& b) const { return Vector3(x / b.x, y / b.y, z / b.z); }

        // dot (inner) product
        Float dot(const Vector3& b) const { return x * b.x + y * b.y + z * b.z; }

        // Cross Product	
        Vector3 operator%(const Vector3& b) const
        {
            return Vector3(
                y * b.z - z * b.y,
                z * b.x - x * b.z,
                x * b.y - y * b.x
            );
        }

        ///! Caution 
        ///! normalize itself
        ///! Caution 
        Vector3& norm() { return *this = *this * (1.0 / sqrt(x*x + y * y + z * z)); }

        // Handy component indexing 
        Float& operator[](const unsigned int i) { return (&x)[i]; }
        const Float& operator[](const unsigned int i) const { return (&x)[i]; }

        std::string str() const
        {
            std::ostringstream ss;
            ss << std::setprecision(5) << "[" << x << ", " << y << ", " << z << "]";
            return ss.str();
        }
    };

    inline Vector3 operator*(Float a, const Vector3& b)
    {
        return b * a;
    }

    // Component-wise cmin
    inline Vector3 cmin(const Vector3& a, const Vector3& b)
    {
        return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
    }
    // Component-wise cmax
    inline Vector3 cmax(const Vector3& a, const Vector3& b)
    {
        return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
    }

    inline Float maxComponet(const Vector3& a)
    {
        return std::max(a.x, std::max(a.y, a.z));
    }

    inline Float minComponet(const Vector3& a)
    {
        return std::min(a.x, std::min(a.y, a.z));
    }

    // Length of a vector
    inline Float length(const Vector3& a)
    {
        return sqrt(a.dot(a));
    }

    // Make a vector unit length
    inline Vector3 normalize(const Vector3& in)
    {
        return in / length(in);
    }

	struct Ray
	{
		Vector3 o; // Ray Origin
		Vector3 d; // Ray Direction
		Vector3 inv_d; // Inverse of each Ray Direction component

		Ray(const Vector3& o, const Vector3& d)
			: o(o), d(d), inv_d(Vector3(1, 1, 1).cdiv(d)) { }
	};


}

#endif
