#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include "interface.h"
#include "Stopwatch.h"
#include "objparser.h"

extern double erand48(unsigned short xseed[3]);

namespace smallpt
{

#ifndef M_PI
#define M_PI  3.1415926
#endif


#if 0
	struct Vec
	{
		double x, y, z;  // position, also color (r,g,b)
		Vec(double x_ = 0, double y_ = 0, double z_ = 0) { x = x_; y = y_; z = z_; }

		Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
		Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
		Vec operator*(double b) const { return Vec(x*b, y*b, z*b); }
		Vec mult(const Vec &b) const { return Vec(x*b.x, y*b.y, z*b.z); }
		Vec& norm() { return *this = *this * (1 / sqrt(x*x + y * y + z * z)); }
		double dot(const Vec &b) const { return x * b.x + y * b.y + z * b.z; }
		Vec operator%(Vec&b) { return Vec(y*b.z - z * b.y, z*b.x - x * b.z, x*b.y - y * b.x); } // cross:
	};
#else
	typedef double Float;

	struct Vector3
	{
		Float x, y, z;

		Vector3(Float x = 0, Float y = 0, Float z = 0) : x(x), y(y), z(z) { }
		Vector3(const Vector3 &b) : x(b.x), y(b.y), z(b.z) { }

		Vector3 operator+(const Vector3& b) const { return Vector3(x + b.x, y + b.y, z + b.z); }
		Vector3 operator-(const Vector3& b) const { return Vector3(x - b.x, y - b.y, z - b.z); }
		Vector3 operator*(Float b) const { return Vector3(x*b, y*b, z*b); }
		Vector3 operator/(Float b) const { b = 1.f / b; return Vector3(x*b, y*b, z*b); }

		Vector3 operator/(const Vector3 &b) const { return Vector3(x / b.x, y / b.y, z / b.z); }

		Vector3 operator+=(const Vector3& b) { return *this = Vector3(x + b.x, y + b.y, z + b.z); }
		Vector3 operator*=(Float b) { return *this = Vector3(x*b, y*b, z*b); }

		// Component-wise multiply and divide
		Vector3 cmul(const Vector3& b) const { return Vector3(x*b.x, y*b.y, z*b.z); }
		Vector3 cdiv(const Vector3& b) const { return Vector3(x / b.x, y / b.y, z / b.z); }
		Vector3 mult(const Vector3& b) const { return Vector3(x*b.x, y*b.y, z*b.z); }

		// dot (inner) product
		Float operator*(const Vector3& b) const { return x * b.x + y * b.y + z * b.z; }
		Float dot(const Vector3& b) const { return x * b.x + y * b.y + z * b.z; }

		// Cross Product	
		Vector3 operator^(const Vector3& b) const
		{
			return Vector3(
				y * b.z - z * b.y,
				z * b.x - x * b.z,
				x * b.y - y * b.x
			);
		}
		Vector3 operator%(const Vector3& b) const
		{
			return Vector3(
				y * b.z - z * b.y,
				z * b.x - x * b.z,
				x * b.y - y * b.x
			);
		}
		Vector3 cross(const Vector3& b) const
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
	};

	inline Vector3 operator*(Float a, const Vector3& b) { return b * a; }

	// Component-wise min
	inline Vector3 min(const Vector3& a, const Vector3& b)
	{
		return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
	}

	inline Vector3 minVector3(const Vector3& a, const Vector3& b)
	{
		return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
	}

	// Component-wise max
	inline Vector3 max(const Vector3& a, const Vector3& b)
	{
		return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
	}

	// Component-wise max
	inline Vector3 maxVector3(const Vector3& a, const Vector3& b)
	{
		return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
	}


	// Length of a vector
	inline Float length(const Vector3& a)
	{
		return sqrt(a*a);
	}

	// Make a vector unit length
	inline Vector3 normalize(const Vector3& in)
	{
		return in / length(in);
	}
#endif


	static const Float inf = 1e20;
	static const Float eps = 1e-6;

    //! Generate a random float in [0, 1)
    Float randomFloat(uint32_t &X);


	inline Float minFloat(Float a, Float b) { return a < b ? a : b; }
	inline Float maxFloat(Float a, Float b) { return a > b ? a : b; }

	enum Refl_t { DIFF, SPEC, REFR };  // material types

	struct Ray
	{
		Vector3 o; // Ray Origin
		Vector3 d; // Ray Direction
		Vector3 inv_d; // Inverse of each Ray Direction component

		Ray(const Vector3& o, const Vector3& d)
			: o(o), d(d), inv_d(Vector3(1, 1, 1).cdiv(d)) { }
	};

	class Object;

	struct IntersectionInfo
	{
		Float t; //< hit distance along the ray
		const Object* object; //< hit object
		Vector3 hit; //< hit point
		IntersectionInfo() : t(inf), object(nullptr), hit() {}
	};

    struct hitable
    {
       virtual bool intersec(const Ray&r, IntersectionInfo& hit) const = 0;
    };

	struct AABB
	{
		inline AABB() { min = Vector3(inf, inf, inf); max = Vector3(-inf, -inf, -inf); }	// an empty interval
		inline AABB(Vector3 min_, Vector3 max_) { min = min_; max = max_; }
		inline bool unbounded() const { return min.x == -inf || min.y == -inf || min.z == -inf || max.x == inf || max.y == inf || max.z == inf; }
		inline size_t largestDimension() const
		{
			double dx = std::fabs(max.x - min.x);
			double dy = std::fabs(max.y - min.y);
			double dz = std::fabs(max.z - min.z);
			if (dx > dy && dx > dz)
			{
				return 0;
			}
			if (dy > dz)
			{
				return 1;
			}
			return 2;
		}

		// ray-slab tests, see PBRT 2nd edition, section 4.2.1
		inline bool intersect(const Ray& ray, const Vector3& inverseDirection, double closestKnownT) const
		{
			bool xDirNegative = ray.d.x < 0;
			bool yDirNegative = ray.d.y < 0;
			bool zDirNegative = ray.d.z < 0;

			// check for ray intersection against x and y slabs
			float tmin = ((xDirNegative ? max.x : min.x) - ray.o.x) * inverseDirection.x;
			float tmax = ((xDirNegative ? min.x : max.x) - ray.o.x) * inverseDirection.x;
			float tymin = ((yDirNegative ? max.y : min.y) - ray.o.y) * inverseDirection.y;
			float tymax = ((yDirNegative ? min.y : max.y) - ray.o.y) * inverseDirection.y;
			if (tmin > tymax || tymin > tmax) {
				return false;
			}
			if (tymin > tmin) {
				tmin = tymin;
			}
			if (tymax < tmax) {
				tmax = tymax;
			}

			// check for ray intersection against z slab
			float tzmin = ((zDirNegative ? max.z : min.z) - ray.o.z) * inverseDirection.z;
			float tzmax = ((zDirNegative ? min.z : max.z) - ray.o.z) * inverseDirection.z;
			if (tmin > tzmax || tzmin > tmax) {
				return false;
			}
			if (tzmin > tmin) {
				tmin = tzmin;
			}
			if (tzmax < tmax) {
				tmax = tzmax;
			}
			return (tmin < closestKnownT) && (tmax > eps);
		}

		Vector3 min;
		Vector3 max;
	};

	struct Object
	{
	public:
		Vector3 e; //< emission
		Vector3 c; //< color
		Refl_t refl; //< reflection type (DIFFuse, SPECular, REFRactive)

	public:
		Object(Vector3 emi = Vector3(), Vector3 color = Vector3(), Refl_t type = DIFF)
			: e(emi), c(color), refl(type) { }
	public:
		//! All "Objects" must be able to test for intersections with rays.
		virtual bool getIntersection(const Ray& ray, IntersectionInfo* intersection) const = 0;

		//! Return an object normal based on an intersection
		virtual Vector3 getNormal(const IntersectionInfo& I) const = 0;

		//! Return a bounding box for this object
		virtual AABB getBBox() const = 0;

		//! Return the centroid for this object. (Used in BVH Sorting)
		virtual Vector3 getCentroid() const = 0;
	};

	struct Sphere : public Object
	{
		Float rad; // radius
		Vector3 p; // position

		Sphere(Float rad_, Vector3 p_, Vector3 e_, Vector3 c_, Refl_t refl_)
			: Object(e_,c_,refl_)
			,rad(rad_), p(p_)
		{ }

		// returns distance, 0 if nohit
		Float intersect(const Ray &r) const
		{
			Vector3 op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
			Float t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
			if (det < 0) return 0; else det = sqrt(det);
			return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
		}
		bool getIntersection(const Ray& ray, IntersectionInfo* I) const override
		{
			Float t = intersect(ray);
			I->object = this;
			I->t = t;
			//I->hit = ray.o + t * ray.d; // derfer to bvh
			return t > 0 ? true : false;
		}
		Vector3 getNormal(const IntersectionInfo& I) const override
		{
			return normalize(I.hit - p);
		}
		AABB getBBox() const override
		{
			return AABB(p - Vector3(rad, rad, rad), p + Vector3(rad, rad, rad));
		}
		Vector3 getCentroid() const override
		{
			return p;
		}
	};

    class SphereScene : public hitable
    {
    private:
        std::vector<Object*> _prims;
        bool _initialized;

    public:
		SphereScene() :  _initialized(false) { init(); }
        bool initialized() const { return _initialized; }
		void init();
        void initScene(Sphere* scene, int n)
        {
            _prims.clear();
            for (int i = n; i--; )
            {
                _prims.push_back(&scene[i]);
            }
            _initialized = true;
        }

        bool intersec(const Ray&r, IntersectionInfo& hit) const override;
    };


    //! only position needed 
    struct TriangleFace
    {
        int v[3]; //< vertex indices
        TriangleFace() { v[0] = v[1] = v[2] = 0; }
        TriangleFace(int x, int y, int z) { v[0] = x; v[1] = y; v[2] = z; }
    };

    struct TriangleMesh
    {
        std::vector<Vector3> verts;
        std::vector<TriangleFace> faces;
        Vector3 bounding_box[2];
    };

    class ObjParser
    {
    private:
        std::string  _filepath;
        TriangleMesh _mesh;
        ObjFile _objModel;

        void loadObj();
        void unitTriangle()
        {
            _mesh.verts.push_back(Vector3(0,1,0));
            _mesh.verts.push_back(Vector3(-1,0,0));
            _mesh.verts.push_back(Vector3(0,0,1));
            _mesh.verts.push_back(Vector3(1,0,0));
            _mesh.verts.push_back(Vector3(0,0,-1));
            //_mesh.faces.push_back(TriangleFace(0, 1, 2));
            _mesh.faces.push_back({ 0, 1, 2 });
            _mesh.faces.push_back({ 0, 2, 3 });
            _mesh.faces.push_back({ 0, 3, 4 });
            _mesh.faces.push_back({ 0, 4, 1 });
        }

    public :
        ObjParser() :_filepath("")
        {
            unitTriangle();
        }

        ObjParser(std::string file) :_filepath(file)
        {
            loadObj();
        }

        const TriangleMesh& getTriangleMesh() const { return _mesh; }
              TriangleMesh& getTriangleMesh()       { return _mesh; }
    };

    class Triangle :public Object
    {
    private:
        Vector3 _v0, _v1, _v2, _e1, _e2;
    public:
        Triangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2 )
            : Object(Vector3(), Vector3() )
			, _v0(v0), _v1(v1), _v2(v2)
            , _e1(v1 - v0)
            , _e2(v2 - v0)
		{
			c *= 0.09;
		}
        Triangle(const Triangle &b)
            : Object(b.e, b.c, b.refl)
			, _v0(b._v0), _v1(b._v1), _v2(b._v2)
            , _e1(b._e1)
            , _e2(b._e2)
        { }

        Vector3 getTriangleNormal()const { return (_e1%_e2).norm(); }

        /////////////////////////////////////////////
        ////http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
        /////////////////////////////////////////////
        Float intersect(const Ray &r) const
        {
            Vector3 tvec = r.o - _v0;
            Vector3 pvec = r.d.cross(_e2);
            Float  det = _e1.dot(pvec);
            //if (det < 1e-5) { return 0;  }//< parallel or backface 
            if (fabs(det) < 1e-5) { return 0;  }//< parallel
            //if (det < 1e-5) { return 0;  }//< parallel or backface 
            Float invdet = 1.0 / det;

            Float u = tvec.dot(pvec) * invdet;
            if (u < 0.0f || u > 1.0f) { return 0; }//< outside triangle

            Vector3 qvec = tvec % _e1;
            Float v = r.d.dot(qvec) * invdet;
            if (v < 0.0f || (u + v) > 1.0f) { return 0; } //< outside triangle

            return _e2.dot(qvec) * invdet;
        }

		bool getIntersection(const Ray& ray, IntersectionInfo* I) const override
		{
			Float t = intersect(ray);
			I->object = this;
			I->t = t;
			//deferto bvh
			//I->hit = ray.o + t * ray.d;
			return t > 0 ? true : false;
		}
		Vector3 getNormal(const IntersectionInfo& I) const override
		{
			return getTriangleNormal();
		}
		AABB getBBox() const override
		{
			return AABB();
		}
		Vector3 getCentroid() const override
		{
			return (_v0+_v1+_v2) / 3.;
		}
    };

    class TriangleScene : public hitable
    {
    private:
	    int     _numTriangles;
	    Vector3	_aabb_min;
	    Vector3	_aabb_max;
        Float   _scale;
        Vector3 _translate;
        std::vector<Triangle> _triangles;

	    void initTriangleScene();

    public:
        TriangleScene()
            :_scale(16.1), _translate(55, 16, 90)
        {
	        initTriangleScene();
        }

        bool intersec(const Ray& r, IntersectionInfo &hit) const override;
    };

    class Scene
    {
    private:
        bool _initialized;
		SphereScene _spheres;
		TriangleScene _triangles;

        bool intersec(const Ray& r, IntersectionInfo &hit) const;

    public:
		Scene() : _initialized(false) { init(); }

		bool init();
        bool initialized() const { return _initialized; }

		Vector3 myradiance(const Ray &r, int depth, unsigned short *Xi);

    };

	// resize screen size
	// reset sample number
	class smallptTest : public Observer
	{
	private:
		int w, h;
		int samples;
		int iterates;
		Vector3 *c;
		float *data;
		int runTest;
		std::ostringstream ss;
		std::string progress;
		Scene scene;

	public:
		smallptTest(int width = 1280, int height = 720, int sample = 1);
		~smallptTest()
		{
			delete c;
			delete data;
		}

		float* renderResult() const { return data; }
		std::string renderLog() const { return ss.str(); }
		std::string renderProgress() const { return progress; }

		void clearRenderLog() { ss.clear(); }

		void run()
		{
			if (runTest)
			{
				smallpt();
			}
		}
	private:
		float* smallpt();

	public:
		virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handlePositionOffsetChange(Float newPositionOffset)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleObjectNumChange(int newObjectNum)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleTestIndexChange(int newTestIndex)
		{
			assert(0 && "This function should NOT be called before override !!!");
		}
		virtual void handleSampleCountChange(int sample)
		{
			if (this->samples != sample / 4)
			{
				this->samples = sample / 4;
				memset(data, 0, sizeof(data));
				memset(c, 0, sizeof(c));
				iterates = 0;
				runTest = true;
			}
		}

	};

}
