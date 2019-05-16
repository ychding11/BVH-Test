#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include "interface.h"
#include "Vector3.h"
#include "Ray.h"
#include "Object.h"
#include "BVH.h"
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
#endif


    //! Generate a random float in [0, 1)
    Float randomFloat(uint32_t &X);

    struct hitable
    {
       virtual bool intersec(const Ray&r, IntersectionInfo& hit) const = 0;
    };

	inline Float minFloat(Float a, Float b) { return a < b ? a : b; }
	inline Float maxFloat(Float a, Float b) { return a > b ? a : b; }

	enum Refl_t { DIFF, SPEC, REFR };  // material types

	struct Sphere : public Object
	{
		Float rad;            // radius
		Vector3 p, e, c;      // position, emission, color
		Refl_t refl;          // reflection type (DIFFuse, SPECular, REFRactive)

		Sphere(Float rad_, Vector3 p_, Vector3 e_, Vector3 c_, Refl_t refl_)
			: rad(rad_), p(p_), e(e_), c(c_), refl(refl_) { }

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
			//I->hit = ray.o + t * ray.d;
			// derfer to bvh
			return t > 0 ? true : false;
		}
		Vector3 getNormal(const IntersectionInfo& I) const override
		{
			return normalize(I.hit - p);
		}
		BBox getBBox() const override
		{
			return BBox(p - Vector3(rad, rad, rad), p + Vector3(rad, rad, rad));
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
        BVH* _bvh;
        bool _initialized;
    public:
		SphereScene() : _bvh(nullptr), _initialized(false) { init(); }
        bool initialized() const { return _initialized; }
		void init();
        void initScene(Sphere* scene, int n)
        {
            _prims.clear();
            _prims.reserve(n);
            for (int i = n; i--; )
            {
                _prims.push_back(&scene[i]);
            }
            _bvh = new BVH(&_prims);
            _initialized = true;
        }

        bool intersec(const Ray&r, IntersectionInfo& hit) const override;
    };

	struct Box
    {

		Vector3 min; // minimum bounds
		Vector3 max; // maximum bounds
		Vector3 emi; // emission
		Vector3 col; // colour
		Refl_t refl; // material type

		// ray/box intersection
		// for theoretical background of the algorithm see 
		// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
		// optimised code from http://www.gamedev.net/topic/495636-raybox-collision-intersection-point/
		Float intersect(const Ray &r) const
		{
			Float epsilon = 1e-3;

			Vector3 tmin = (min - r.o) / r.d;
			Vector3 tmax = (max - r.o) / r.d;

			Vector3 real_min = minVector3(tmin, tmax);
			Vector3 real_max = maxVector3(tmin, tmax);

			Float minmax = minFloat(minFloat(real_max.x, real_max.y), real_max.z);
			Float maxmin = maxFloat(maxFloat(real_min.x, real_min.y), real_min.z);

			if (minmax >= maxmin) { return maxmin > epsilon ? maxmin : 0; }
			else return 0;
		}

		// calculate normal for point on axis aligned box
		Vector3 normalAt(Vector3 &point)
		{

			Vector3 normal(0.f, 0.f, 0.f);
			Float min_distance = 1e8;
			Float distance;
			Float epsilon = 0.001f;

			if (fabs(min.x - point.x) < epsilon) normal = Vector3(-1, 0, 0);
			else if (fabs(max.x - point.x) < epsilon) normal = Vector3(1, 0, 0);
			else if (fabs(min.y - point.y) < epsilon) normal = Vector3(0, -1, 0);
			else if (fabs(max.y - point.y) < epsilon) normal = Vector3(0, 1, 0);
			else if (fabs(min.z - point.z) < epsilon) normal = Vector3(0, 0, -1);
			else normal = Vector3(0, 0, 1);

			return normal;
		}
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
		Vector3 e, c;      // emission, color
		Refl_t refl;       // reflection type (DIFFuse, SPECular, REFRactive)
    public:
        Triangle(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2 )
            : _v0(v0), _v1(v1), _v2(v2)
            , _e1(v1 - v0)
            , _e2(v2 - v0)
			, e(0,0,0), c(1,1,1)
			, refl(DIFF)
		{
			c *= 0.09;
		}
        Triangle(const Triangle &b)
            : _v0(b._v0), _v1(b._v1), _v2(b._v2)
            , _e1(b._e1)
            , _e2(b._e2)
			, e(b.e), c(b.c)
			, refl(b.refl)
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
		BBox getBBox() const override
		{
			return BBox(Vector3(), Vector3());
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
