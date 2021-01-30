#pragma once

#include <vector>
#include <sstream>
#include <string>
#include <iostream>

#include "BVH.h"
#include "interface.h"
#include "FastNoise.h"
#include "Profiler.h"

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

class BVHTracer : public Observer
{
public:

    BVHTracer(int n, int width, int height, std::ostringstream& logger)
		: _scale (n)
        , _objectNum(_scale * _scale * _scale)
        , _width(width), _height(height)
        , _traceDone(false)
		, _cameraPositionOffset(0.f)
		, _focusOffset(0.f, 0.f)
        , _bvh(nullptr)
        , _objects(MAX_OBJECT_NUM)
		, _randomVectors(MAX_OBJECT_NUM)
		, _noise()
		, _logger(logger)
    {
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
	int _scale;
    int _objectNum;
    int _width;
    int _height;
    bool _traceDone;
	float _cameraPositionOffset;
	glm::fvec2 _focusOffset;

	BVH* _bvh;
    std::vector<Object*> _objects;
    std::vector<Vector3*> _randomVectors;
	std::ostringstream& _logger;
	FastNoise _noise; // Create a FastNoise object

    void constructBVH();
    
public:

	virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override
    {
        if (newScreenSize.x != _width || newScreenSize.y != _height)
        {
            _width = newScreenSize.x;
			_height = newScreenSize.y;
            delete _pixels;
            _pixels = new float[_width * _height * 3];
            _traceDone = false;
        }
    }
	virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset) override
    {
		if (newFocusOffset != _focusOffset)
		{
			_focusOffset = newFocusOffset;
			_traceDone = false;
		}
    }
	virtual void handlePositionOffsetChange(float newPositionOffset) override
    {
		if (newPositionOffset != _cameraPositionOffset)
		{
			_cameraPositionOffset = newPositionOffset;
			_traceDone = false;
		}
    }
	virtual void handleObjectNumChange(int newScale) override
    {
        if (newScale!= _scale)
        {
			if (newScale * newScale * newScale > MAX_OBJECT_NUM)
			{
				_logger << "Object Number exceeds max value " << MAX_OBJECT_NUM << std::endl;
				return;
			}
            _objectNum = newScale * newScale * newScale;
			_scale = newScale;
            constructBVH();
            _traceDone = false;
        }
    }

	int objectNum() const
	{
		return _objectNum;
	}

    void run()
    {
        if (_traceDone == false)
        {
            trace();
            _traceDone = true;
        }
    }

private:
	void trace();
};

