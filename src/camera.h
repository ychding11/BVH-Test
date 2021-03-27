#ifndef _camera_h
#define _camera_h

#include <bvh/vector.hpp> 
#include <string> 
#include <sstream> 

#include "Log.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

struct Camera
{
    Vector3 eye;
    Vector3 dir;
    Vector3 up;
    Scalar  fov;

    std::string str() const
    {
        std::stringstream ss;

        ss
        << "camera info: "
        << "eye: [ " << eye[0] << "," << eye[1] << "," << eye[2] << " ]\n"
        << "dir: [ " << dir[0] << "," << dir[1] << "," << dir[2] << " ]\n"
        << "eye: [ " << up[0]  << "," << up[1]  << "," << up[2]  << " ]\n"
        << "fov: [ " << fov << " ]\n" ;

        return ss.str();
    }
};

template<typename OStream>
inline OStream& operator<<(OStream &os, const Camera& c)
{
    return os << "camera info: \n"
        << "\t eye: [ " << c.eye[0] << "," << c.eye[1] << "," << c.eye[2] << " ]\n"
        << "\t dir: [ " << c.dir[0] << "," << c.dir[1] << ","  << c.dir[2] << " ]\n"
        << "\t eye: [ " << c.up[0] << "," << c.up[1] << "," << c.up[2] << " ]\n"
        << "\t fov: [ " << c.fov << " ]\n" ;
}

#endif
