#pragma once

#include "raytracer.h"


    void BVHTracer::constructBVH()
    {
        _objects.clear();
        delete _bvh;

        {
            CPUProfiler profiler("generate objects", true);
            for (size_t i = 0; i < _objectPerAxis; ++i)
            for (size_t j = 0; j < _objectPerAxis; ++j)
            for (size_t k = 0; k < _objectPerAxis; ++k)
            {    
                float invScale = 1.f / _objectPerAxis;
                float r = 0.005f;
                float x = (rand01() - 0.5) * 0.25;
                float y = (rand01() - 0.5) * 0.25;
                float z = (rand01() - 0.5) * 0.25;
                Vector3 c( (i + 0.5f + x) * invScale, (j + 0.5f + y) * invScale, (k + 0.5f + z) * invScale );
                _objects.push_back(new Sphere(c, r));
            }
        }

        {
            CPUProfiler profiler("construct bvh", true);
            _bvh = new BVH(&_objects);
        }
    }
    
    void BVHTracer::trace()
    {
        CPUProfiler profiler("trace bvh", true);

        // Create a camera from position and focus point
        Vector3 camera_position(1.6, 1.3, 1.6);
        Vector3 camera_focus(0, 0, 0);
        Vector3 camera_up(0, 1, 0);

        // Camera tangent space
        Vector3 camera_dir = normalize(camera_focus - camera_position);
        Vector3 camera_u = normalize(camera_dir ^ camera_up);
        Vector3 camera_v = normalize(camera_u ^ camera_dir);
        camera_position = camera_position + camera_dir * _cameraPositionOffset; // UI controled

        // Raytrace over every pixel
        #pragma omp parallel for
        for (size_t i = 0; i< _width; ++i)
        {
            for (size_t j = 0; j< _height; ++j)
            {
                size_t index = 3 * (_width * j + i);

                float u = (i + .25f) / (float)(_width) - .5f;
                float v = (_height - 1 - j + .25f) / (float)(_height) - .5f;
                float fovlen = .5f / tanf(70.f * 3.14159265*.5f / 180.f);

                // This is only valid for square images
                Ray ray(camera_position, normalize(u * camera_u + v * camera_v + fovlen * camera_dir));

                IntersectionInfo I;
                bool hit = _bvh->getIntersection(ray, &I, false);

                if (!hit)
                {
                    _pixels[index] = _pixels[index + 1] = _pixels[index + 2] = 0.f;
                }
                else
                {
                    // Just for fun, we'll make the color based on the normal
                    const Vector3 normal = I.object->getNormal(I);
                    const Vector3 color(fabs(normal.x), fabs(normal.y), fabs(normal.z));

                    _pixels[index] = color.x;
                    _pixels[index + 1] = color.y;
                    _pixels[index + 2] = color.z;
                }
            }
        }
    }

