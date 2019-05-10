#include <math.h>   // smallpt, a Path Tracer by Kevin Beason, 2008
#include <stdlib.h> // Make : g++ -O3 -fopenmp smallpt.cpp -o smallpt
#include <stdio.h>  //        Remove "-fopenmp" for g++ version < 4.2
#include <iostream>
#include <sstream>

#include "interface.h"

#ifndef M_PI
#define M_PI  3.1415926
#endif

struct Vec
{
  double x, y, z;  // position, also color (r,g,b)
  Vec(double x_=0, double y_=0, double z_=0){ x=x_; y=y_; z=z_; }

  Vec operator+(const Vec &b) const { return Vec(x+b.x,y+b.y,z+b.z); }
  Vec operator-(const Vec &b) const { return Vec(x-b.x,y-b.y,z-b.z); }
  Vec operator*(double b) const { return Vec(x*b,y*b,z*b); }
  Vec mult(const Vec &b) const { return Vec(x*b.x,y*b.y,z*b.z); }
  Vec& norm(){ return *this = *this * (1/sqrt(x*x+y*y+z*z)); }
  double dot(const Vec &b) const { return x*b.x+y*b.y+z*b.z; }
  Vec operator%(Vec&b){return Vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);} // cross:
};


// resize screen size
// reset sample number
class smallptTest : public Observer
{
private:
	int w, h;
	int samples;
	int iterates;
	Vec *c ;
	float *data;
	int runTest;
	std::ostringstream ss;
	std::string progress;

public:
	smallptTest(int width = 1280, int height = 720, int sample = 1)
		: w(width), h(height), samples(sample), iterates(0)
		, runTest(true)
	{
		c = new Vec[w * h];
		data = new float[w * h * 3];
		this->handleSampleCountChange(sample);
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
	virtual void handlePositionOffsetChange(float newPositionOffset)
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

