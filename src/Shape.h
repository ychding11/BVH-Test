#ifndef Sphere_h_
#define Sphere_h_

#include <cmath>
#include "Vector3.h"

struct Ray
{
    Vector3 o; // Ray Origin
    Vector3 d; // Ray Direction
    Vector3 inv_d; // Inverse of each Ray Direction component

    Ray(const Vector3& o, const Vector3& d) : o(o), d(d), inv_d(Vector3(1,1,1).cdiv(d)) { }
};

struct BBox
{
    Vector3 min, max, extent;
    BBox() { }

    BBox(const Vector3& min, const Vector3& max) : min(min), max(max) { extent = max - min; }

    BBox(const Vector3& p) : min(p), max(p) { extent = max - min; }
 
    void expandToInclude(const Vector3& p)
    {
        min = ::min(min, p);
        max = ::max(max, p);
        extent = max - min;
    }

    void expandToInclude(const BBox& b)
    {
        min = ::min(min, b.min);
        max = ::max(max, b.max);
        extent = max - min;
    }

    uint32_t maxDimension() const
    {
        uint32_t result = 0;
        if(extent.y > extent.x) result = 1;
        if(extent.z > extent.y) result = 2;
        return result;
    }

    float surfaceArea() const { return 2.f*( extent.x*extent.z + extent.x*extent.y + extent.y*extent.z ); }

    // Typical slab-based Ray-AABB test
    bool intersect(const Ray& ray, float *tnear, float *tfar) const
    {
        Vector3 tbot = ray.inv_d.cmul(min - ray.o);
        Vector3 ttop = ray.inv_d.cmul(max - ray.o);
	 
        Vector3 tmin = ::min(ttop, tbot);
        Vector3 tmax = ::max(ttop, tbot);

        *tnear = std::max(std::max(tmin.x, tmin.y), tmin.z);
        *tfar = std::min(std::min(tmax.x,tmax.y), tmax.z);

        return !(*tnear > *tfar) && *tfar > 0;
    }

};

struct Object;

struct IntersectionInfo
{
    float t; // Intersection distance along the ray
    const Object* object; // Object that was hit
    Vector3 hit; // Location of the intersection
};

struct Object
{
     //! All "Objects" must be able to test for intersections with rays.
     virtual bool getIntersection( const Ray& ray, IntersectionInfo* intersection) const = 0; 

     //! Return an object normal based on an intersection
     virtual Vector3 getNormal(const IntersectionInfo& I) const = 0;

     //! Return a bounding box for this object
     virtual BBox getBBox() const = 0;

     //! Return the centroid for this object. (Used in BVH Sorting)
     virtual Vector3 getCentroid() const = 0;
};

struct Sphere : public Object
{
    Vector3 center; // Center of the sphere
    float r, r2;    // Radius, Radius^2

    Sphere(const Vector3& center, float radius) : center(center), r(radius), r2(radius*radius) { }

    bool getIntersection(const Ray& ray, IntersectionInfo* I) const override
    {
        Vector3 s = center - ray.o;
        float sd = s * ray.d;
        float ss = s * s;
  
        // Compute discriminant
        float disc = sd*sd - ss + r2;

        // Complex values: No intersection
        if( disc < 0.f ) return false; 

        // Assume we are not in a sphere... The first hit is the lesser valued
        I->object = this;
        I->t = sd - sqrt(disc);
        return true;
    }
 
    Vector3 getNormal(const IntersectionInfo& I) const override
    {
        return normalize(I.hit - center);
    }

    BBox getBBox() const override
    { 
        return BBox(center-Vector3(r,r,r), center+Vector3(r,r,r)); 
    }

    Vector3 getCentroid() const override
    {
        return center;
    }

};

#endif
