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
		std::ostringstream _ss;

		void constructCoordinate();
		
        //! get a sample in [0,0), support subpixel;
		Float getSample(unsigned short *X = nullptr);
        
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
			constructCoordinate();
			_ss << "camera creation.\t dir:" << _d.str() << "\t pos:" << _p.str() << std::endl;
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
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr);
		
	};

}

#endif
