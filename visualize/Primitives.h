#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "Vector3.h"

namespace smallpt
{
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

    struct AABB
    {
        Vector3 bmin;
        Vector3 bmax;

        AABB()
        {
            bmin = Vector3(inf, inf, inf);
            bmax = Vector3(-inf, -inf, -inf);
        }

        AABB(Vector3 min_, Vector3 max_)
        {
            bmin = min_;
            bmax = max_;
        }

        AABB Union(const AABB& b)
        {
            AABB res;
            res.bmin = cmin(bmin, b.bmin);
            res.bmax = cmax(bmax, b.bmax);
            return res;
        }

        AABB Enclose(const Vector3& p)
        {
            AABB res;
            res.bmin = cmin(bmin, p);
            res.bmax = cmax(bmax, p);
            return res;
        }

        bool hit(const Ray& r) const
        {
            Vector3 t0 = (bmin - r.o).cdiv(r.d);
            Vector3 t1 = (bmax - r.o).cdiv(r.d);

            Vector3 tsmaller = cmin(t0, t1);
            Vector3 tbigger = cmax(t0, t1);

            return maxComponet(tsmaller) < minComponet(tbigger);
        }

    private:
        // from "A Ray-Box Intersection Algorithm and Efficient Dynamic Voxel Rendering"
        // http://jcgt.org/published/0007/03/04/
        // note: ray direction should be inverted, i.e 1.0/direction!
        bool HitAABB(const Ray& r, Float tMin, Float tMax)
        {
            Vector3 t0 = (bmin - r.o).cdiv(r.d);
            Vector3 t1 = (bmax - r.o).cdiv(r.d);

            Vector3 tsmaller = cmin(t0, t1);
            Vector3 tbigger = cmax(t0, t1);

            //tMin = std::max(tMin, maxComponet(tsmaller)); tMax = std::min(tMax, minComponet(tbigger));
            //return tMin <= tMax;
            return maxComponet(tsmaller) < minComponet(tbigger);
        }

    };

    struct Object
    {
    public:
        Vector3 e; //< emission
        Vector3 c; //< color
        Refl_t refl; //< reflection type (DIFFuse, SPECular, REFRactive)

    public:
        Object(Vector3 emi = Vector3(), Vector3 color = Vector3(0.5, 0.5, 0.5), Refl_t type = DIFF)
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
            : Object(e_, c_, refl_)
            , rad(rad_), p(p_)
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

    class Triangle :public Object
    {
    private:
        Vector3 _v0, _v1, _v2, _e1, _e2;
    public:
        Triangle()
            : Object()
			, _v0(), _v1(), _v2()
            , _e1()
            , _e2()
		{ }

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
            Vector3 pvec = r.d % _e2;
            Float  det = _e1.dot(pvec);
            if (fabs(det) < 1e-5) { return 0;  }//< parallel
            //if (det < 1e-5) { return 0;  }//< parallel or backface 
            Float invdet = 1.0 / det;

            Float u = tvec.dot(pvec)* invdet;
            if (u < 0.0f || u > 1.0f) { return 0; }//< outside triangle

            Vector3 qvec = tvec % _e1;
            Float v = r.d.dot(qvec)* invdet;
            if (v < 0.0f || (u + v) > 1.0f) { return 0; } //< outside triangle

            return _e2.dot(qvec)* invdet;
        }

		bool getIntersection(const Ray& ray, IntersectionInfo* I) const override
		{
			Float t = intersect(ray);
            if (t > eps && t < inf)
            {
			    I->object = this;
			    I->t = t;
			    //deferto bvh
			    //I->hit = ray.o + t * ray.d;
                return true;
            }
			return false;
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


}

#endif
