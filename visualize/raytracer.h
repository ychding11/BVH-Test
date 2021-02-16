#pragma once

#include <vector>
#include <sstream>
#include <string>
#include <iostream>

#include "BVH.h"
#include "FastNoise.h"
#include "Profiler.h"

#include "interface.h"

// Return a random number in [0,1]
static float rand01()
{ 
    return rand() * (1.f / RAND_MAX); 
}

// Return a random vector with each component in the range [-1,1]
static Vector3 randVector3()
{
    return Vector3(rand01(), rand01(), rand01()) * 2.f - Vector3(1,1,1);
}

#define MAX_OBJECT_NUM (1000000)

class BVHTracer
{
public:

    BVHTracer(std::ostringstream& logger)
        : _traceDone(false)
        , _cameraPositionOffset(0.f)
        , _bvh(nullptr)
        , _objects(MAX_OBJECT_NUM)
        , _randomVectors(MAX_OBJECT_NUM)
        , _noise()
        , _logger(logger)
    {
        _width = settings.width;
        _height = settings.height;
        _objectPerAxis = settings.objectPerAxis;
        _objectNum = _objectPerAxis * _objectPerAxis * _objectPerAxis;

        _pixels = new float[_width * _height * 3];
        _noise.SetNoiseType(FastNoise::SimplexFractal); // Set the desired noise type
        constructBVH();
    }

    ~BVHTracer()
    {
        delete _pixels;
        delete _bvh;
    }

    float* _pixels;

private:
    int _objectPerAxis;
    int _objectNum;
    int _width;
    int _height;
    bool _traceDone;
    float _cameraPositionOffset;

    BVH* _bvh;
    std::vector<Object*> _objects;
    std::vector<Vector3*> _randomVectors;
    std::ostringstream& _logger;
    FastNoise _noise; // Create a FastNoise object

    void constructBVH();
    
public:

    //virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override
    //{
    //    if (newScreenSize.x != _width || newScreenSize.y != _height)
    //    {
    //        _width = newScreenSize.x;
    //        _height = newScreenSize.y;
    //        delete _pixels;
    //        _pixels = new float[_width * _height * 3];
    //        _traceDone = false;
    //    }
    //}
    //virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset) override
    //{
    //    if (newFocusOffset != _focusOffset)
    //    {
    //        _focusOffset = newFocusOffset;
    //        _traceDone = false;
    //    }
    //}

    int objectNum() const
    {
        return _objectNum;
    }

    float* getResult()
    {
        if (settings.positionOffset != _cameraPositionOffset)
        {
            _cameraPositionOffset = settings.positionOffset;
            _traceDone = false;
        }
        else
        {
        }

        if (settings.objectPerAxis != _objectPerAxis)
        {
            if (settings.objectPerAxis * settings.objectPerAxis * settings.objectPerAxis > MAX_OBJECT_NUM)
            {
                _logger << "Object Number exceeds max value " << MAX_OBJECT_NUM << std::endl;
            }
            else
            {
                _objectNum = settings.objectPerAxis * settings.objectPerAxis * settings.objectPerAxis;
                _objectPerAxis = settings.objectPerAxis;
                constructBVH();
                _logger << "Set Object Number  " << _objectNum << std::endl;
                _traceDone = false;
            }
        }

        if (_traceDone == false)
        {
            trace();
            _traceDone = true;
        }
        return _pixels;
    }

private:
    void trace();
};
