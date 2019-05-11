#ifndef IntersectionInfo_h_
#define IntersectionInfo_h_

#include "Vector3.h"

class Object;

struct IntersectionInfo
{
 Float t; // Intersection distance along the ray
 const Object* object; // Object that was hit
 Vector3 hit; // Location of the intersection
};

#endif
