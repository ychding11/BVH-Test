
#include "camera.h"

namespace smallpt
{
	void Camera::constructCoordinate()
	{
		Float halfVfov = _vfov * 0.5 * (M_PI / 180.);
        Float h = std::tanf(halfVfov);
		Float aspect = double(_w) / double(_h);
		_u = Vector3(aspect * h, 0, 0);
		_v = (_u%_d).norm()*h;
	}

    //! get a sample in [0,0), support subpixel;
    Float Camera::getSample(unsigned short *X)
    {
        static uint32_t i = 0;
        uint32_t N = 2; //< NxN sub-pixel
        Float w = 1. / double(N) * 0.5; //< width of sub-pixel
        Float c = w; //< first center of sub-pixel
        Float rd; //< random uniform distribute [0,1)
        if (X)
        {
            rd = erand48(X);
        }
        else
        {
            rd = randomFloat();
        }
        rd = (2. * rd - 1.) * w;
        c =  (i++) % N * w * 2.;
        return c + rd;
    }

	//! get a random ray based on (u, v) in image plane
	Ray Camera::getRay(uint32_t u, uint32_t v, unsigned short *X )
	{
        Float x = getSample(X);
        Float y = getSample(X);
        Float dW = 1. / double(_w);
        Float dH = 1. / double(_h);
        x = x * dW + double(u) / double(_w) - 0.5;
        y = y * dW + double(v) / double(_w) - 0.5;
        Vector3 d = _u * x + _v * y + _d;
		//Ray r(_p + d * 140, d.norm());
		Ray r(_p, d.norm());
		return r;
	}

}