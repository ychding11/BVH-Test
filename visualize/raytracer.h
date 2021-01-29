#pragma once

#include <vector>
#include <sstream>
#include <string>
#include <iostream>


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "BVH.h"
#include "Sphere.h"
#include "interface.h"
#include "FastNoise.h"
#include "Stopwatch.h"

// Return a random number in [0,1]
static float rand01()
{ 
 return rand() * (1.f / RAND_MAX); 
}

// Return a random vector with each component in the range [-1,1]
static Vector3 randVector3()
{
 return Vector3(rand01(), rand01(), rand01())*2.f - Vector3(1,1,1);
}

class BVHTracer : public Observer
{
public:

    BVHTracer(int n, int width, int height, std::ostringstream& logger)
        : _maxObjectNum(1000000)
		, _scale (n)
        , _objectNum(_scale * _scale * _scale)
        , _width(width), _height(height)
        , _testDone(false)
		, _cameraPositionOffset(0.f)
		, _focusOffset(0.f, 0.f)
        , _bvh(nullptr)
        , _objects(_maxObjectNum)
		, _randomVectors(_maxObjectNum)
		, _noise()
		, _logger(logger)
    {
        _pixels = new float[_width * _height * 3];
	    _noise.SetNoiseType(FastNoise::SimplexFractal); // Set the desired noise type
		for (int i = 0; i < _maxObjectNum; ++i)
		{

		}

        constructBVH();
    }

    ~BVHTracer()
    {
        delete _pixels;
        delete _bvh;
    }

    float* _pixels;

private:
    const int _maxObjectNum;
	int _scale;
    int _objectNum;
    int _width;
    int _height;
    bool _testDone;
	float _cameraPositionOffset;
	glm::fvec2 _focusOffset;

	BVH* _bvh;
    std::vector<Object*> _objects;
    std::vector<Vector3*> _randomVectors;
	FastNoise _noise; // Create a FastNoise object
	std::ostringstream& _logger;

#if 0
    void constructBVH()
    {
        _objects.clear();
        for (size_t i = 0; i < _objectNum; ++i)
        {
            _objects.push_back(new Sphere(randVector3(), .005f));
			//float v = _noise.GetNoise(x, y);
        }

        delete _bvh;
        _bvh = new BVH(&_objects);
    }
#endif
    
    void constructBVH()
    {
        _objects.clear();

		{
			CPUProfiler profiler("generate objects", true);
			for (size_t i = 0; i < _scale; ++i)
			for (size_t j = 0; j < _scale; ++j)
			for (size_t k = 0; k < _scale; ++k)
			{    
				float invScale = 1.f / _scale;
				float r = 0.005f;
				float x = (rand01() - 0.5) * 0.25;
				float y = (rand01() - 0.5) * 0.25;
				float z = (rand01() - 0.5) * 0.25;
				Vector3 c( (i + 0.5f + x) * invScale, (j + 0.5f + y) * invScale, (k + 0.5f + z) * invScale );
				_objects.push_back(new Sphere(c, r));
			}
		}

        delete _bvh;
		{
			CPUProfiler profiler("construct bvh", true);
			_bvh = new BVH(&_objects);
		}
    }
    
public:

	virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override
    {
        if (newScreenSize.x != _width || newScreenSize.y != _height)
        {
            _width = newScreenSize.x;
			_height = newScreenSize.y;
            delete _pixels;
            _pixels = new float[_width * _height * 3];
            _testDone = false;
        }
    }
	virtual void handleFocusOffsetChange(const glm::fvec2 &newFocusOffset) override
    {
		if (newFocusOffset != _focusOffset)
		{
			_focusOffset = newFocusOffset;
			_testDone = false;
		}
    }
	virtual void handlePositionOffsetChange(float newPositionOffset) override
    {
		if (newPositionOffset != _cameraPositionOffset)
		{
			_cameraPositionOffset = newPositionOffset;
			_testDone = false;
		}
    }
	virtual void handleObjectNumChange(int newScale) override
    {
        if (newScale!= _scale)
        {
			if (newScale * newScale * newScale > _maxObjectNum)
			{
				_logger << "Object Number exceeds max value " << _maxObjectNum << std::endl;
				return;
			}
            _objectNum = newScale * newScale * newScale;
			_scale = newScale;
            constructBVH();
            _testDone = false;
        }
    }
	virtual void handleTestIndexChange(int newTestIndex) override
    {
        assert(0 && "This function should NOT be called before override !!!");
    }

	int objectNum() const
	{
		return _objectNum;
	}

#if 0
    void objectNum(int n)
    {
        if (n != _objectNum)
        {
            _objectNum = n;
            constructBVH();
            _testDone = false;
        }
    }
    void screenSize(int x, int y)
    {
        if (x != _width || y != _height)
        {
            _width = x; _height = y;
            delete _pixels;
            _pixels = new float[_width * _height * 3];
            _testDone = false;
        }
    }
#endif
   
    void run()
    {
        if (_testDone == false)
        {
            trace();
            _testDone = true;
        }
    }

private:
	void trace();
};

