#pragma once

#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <iostream>
#include <sstream>
#include <random>
#include <fstream>

#include "interface.h"
#include "Vector3.h"
#include "Ray.h"
#include "Object.h"
#include "BVH.h"

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

	static std::default_random_engine generator;
	static std::uniform_real_distribution<Float> distr(0.0, 1.0);
	static Float erand48(unsigned short* X = 0)
	{
		return distr(generator);
	}


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

	struct Box {

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

    // helpers to load triangle data
    struct TriangleFace
    {
        int v[3]; // vertex indices
    };

    struct TriangleMesh
    {
        std::vector<Vector3> verts;
        std::vector<TriangleFace> faces;
        Vector3 bounding_box[2];
    };

	//TriangleMesh mesh1;
	//TriangleMesh mesh2;
	//std::vector<Vector3> triangles;
	extern int total_number_of_triangles;
	extern Vector3	scene_aabbox_min;
	extern Vector3	scene_aabbox_max;

	void initTriangleScene();
	Vector3 radiance(Ray &r, const int totaltris, unsigned short *Xi, const Vector3& scene_aabb_min, const Vector3& scene_aabb_max);
}
