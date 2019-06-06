#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include <thread>
#include <mutex>
#include <condition_variable>

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
    Float randomFloat();

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

	//! Generate rays easly
	class Camera
	{
	private:
		Float _vfov;
		uint32_t _w, _h;
		Vector3 _p, _u, _v, _d; //< coordiate & position

		void constructCoordinate()
		{
			Float halfVfov = _vfov * 0.5 * (M_PI / 180.);
            Float h = std::tanf(halfVfov);
			Float aspect = double(_w) / double(_h);
			_u = Vector3(aspect * h, 0, 0);
			_v = (_u%_d).norm()*h;
		}

        //! get a sample in [0,0), support subpixel;
        Float getSample(unsigned short *X = nullptr)
        {
            static uint32_t i = 0;
            uint32_t N = 2; //< NxN sub-pixel
            Float w = 1. / double(N) * 0.5; //< width of sub-pixel
            Float c = w; //< first center of sub-pixel
            Float rd; //< random uniform distribute [0,1)
            if (X)
            {
                rd = erand48(X);
            }
            else
            {
                rd = randomFloat();
            }
            rd = (2. * rd - 1.) * w;
            c =  (i++) % N * w * 2.;
            return c + rd;
        }

	public:
		Camera() = delete; //< No default constructor allowed
		//Camera(uint32_t w, uint32_t _h, Float vfov);

		//! \param position is in world space
		//! \param dir is in world space and is normalized
		Camera(Vector3 position, Vector3 dir, uint32_t w = 1024, uint32_t h = 1024, Float vfov = 55.)
			: _p(position), _d(dir)
			, _w(w), _h(h)
			, _vfov(vfov)
		{
			constructCoordinate();
		}

		void setImageSize(uint32_t w, uint32_t h)
		{
			if (w != _w || h != _h)
			{
				_w = w; _h = h;
				constructCoordinate();
			}
		}

		Float aspect() const { return double(_w) / double(_h); };

		//! get a random ray based on (u, v) in image plane
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr)
		{
            Float x = getSample(X);
            Float y = getSample(X);
            Float dW = 1. / double(_w);
            Float dH = 1. / double(_h);
            x = x * dW + double(u) / double(_w) - 0.5;
            y = y * dW + double(v) / double(_w) - 0.5;
            Vector3 d = _u * x + _v * y + _d;
			Ray r(_p + d * 140, d.norm());
			return r;
		}
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
		Object(Vector3 emi = Vector3(), Vector3 color = Vector3(0.5,0.5,0.5), Refl_t type = DIFF)
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
        std::vector<Float> rawVerts;
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
            : Object()
			, _v0(v0), _v1(v1), _v2(v2)
            , _e1(v1 - v0)
            , _e2(v2 - v0)
		{
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
            :_scale(161), _translate(55, 6, 90)
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
		//Vector3 newradiance(const Ray &r, int depth, unsigned short *Xi);

		//! used for hit algorithms test.
		Vector3 hittest(const Ray &r);

    };

	// resize screen size
	// reset sample number
	class smallptTest : public Observer
	{
	public:
		static void render(void *data);
        static std::mutex _sMutex;

	private:
		int w, h;
		int spp;
		int iterates;
		Vector3 *c;
		float *data;
		int runTest;
		std::ostringstream ss;
		std::string progress;
		Scene scene;

        Camera _camera; // cam pos, dir

		std::mutex _mutex;
		std::condition_variable _condVar;
		std::thread _renderThread;
        bool _exitRendering;
		bool _isRendering; //< rendering thread responsible for this
		bool _pauseRender;

	public:
		smallptTest(int width = 720, int height = 720, int sample = 1);
		~smallptTest()
		{
            _exitRendering = true;
            _renderThread.join(); //< wait render thread exit, then delete resource.
			delete c;
			delete data;
		}

		float* renderResult() const
        {
            std::lock_guard<std::mutex> lock(_sMutex);
            return data;
        }
		std::string renderLog() const { return ss.str(); }
		std::string renderProgress() const { return progress; }

		void clearRenderLog() { ss.clear(); }

		void run()
		{
			if (runTest)
			{
				_condVar.notify_one();
			}
		}
	private:
		void newsmallpt();

	public:
		virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize)
		{
            std::lock_guard<std::mutex> lock(_sMutex);
			_camera.setImageSize(newScreenSize.x, newScreenSize.y);
            w = newScreenSize.x; h = newScreenSize.y;
			delete c; delete data; //< delete nullptr is OK
		    c = new Vector3[w * h]; data = new float[w * h * 3];
			assert((c && data));
			memset(data, 0, sizeof(data));
			memset(c, 0, sizeof(c));
			iterates = 0;
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
			if (this->spp != sample / 4)
			{
				// signal rendering thread stop
				// wait for rendering thread's signal
				this->spp = sample / 4;
				memset(data, 0, sizeof(data));
				memset(c, 0, sizeof(c));
				iterates = 0;
				runTest = true;
			}
		}

	};

}
