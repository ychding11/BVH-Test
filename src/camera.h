#ifndef _camera_h
#define _camera_h

#include <bvh/vector.hpp> 

#include "Log.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

struct Camera
{
    Vector3 eye;
    Vector3 dir;
    Vector3 up;
    Scalar  fov;
};

template<typename OStream>
inline OStream& operator<<(OStream &os, const Camera& c)
{
    return os << "camera info: "
        << "eye: [ " << c.eye[0] << "," << c.eye[1] << "," << c.eye[2] << " ]\n"
        << "dir: [ " << c.dir[0] << "," << c.dir[1] << ","  << c.dir[2] << " ]\n"
        << "eye: [ " << c.up[0] << "," << c.up[1] << "," << c.up[2] << " ]\n"
        << "fov: [ " << c.fov << " ]\n" ;
}

#endif
