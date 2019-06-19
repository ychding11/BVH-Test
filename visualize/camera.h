#ifndef CAMERA_H
#define CAMERA_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>

#include "Vector3.h"
#include "randoms.h"

namespace smallpt
{
	class Camera
	{
	private:
		Float _vfov;
		uint32_t _w, _h;
		Vector3 _p, _u, _v, _d; //< coordiate & position

		void constructCoordinate()
		{
			Float halfVfov = _vfov * 0.5 * (M_PI / 180.);
            Float h = std::tanf(halfVfov);
			Float aspect = double(_w) / double(_h);
			_u = Vector3(aspect * h, 0, 0);
			_v = (_u%_d).norm()*h;
		}

        //! get a sample in [0,0), support subpixel;
        Float getSample(unsigned short *X = nullptr)
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

	public:
		Camera() = delete; //< No default constructor allowed
		//Camera(uint32_t w, uint32_t _h, Float vfov);

		//! \param position is in world space
		//! \param dir is in world space and is normalized
		Camera(Vector3 position, Vector3 dir, uint32_t w = 1024, uint32_t h = 1024, Float vfov = 55.)
			: _p(position), _d(dir)
			, _w(w), _h(h)
			, _vfov(vfov)
		{
			std::cout << "camera dir: " << _d.str() << "\ncamera pos:" << _p.str() << std::endl;
			constructCoordinate();
		}

		void setImageSize(uint32_t w, uint32_t h)
		{
			if (w != _w || h != _h)
			{
				_w = w; _h = h;
				constructCoordinate();
			}
		}

		Float aspect() const { return double(_w) / double(_h); };

		//! get a random ray based on (u, v) in image plane
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr)
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
	};

}

#endif
