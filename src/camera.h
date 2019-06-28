#ifndef CAMERA_H
#define CAMERA_H

#include <math.h>
#include <stdlib.h>
#include <mutex>
#include <iostream>
#include <sstream>

#include "points.h"
#include "Vector3.h"
#include "randoms.h"

namespace mei
{
	// FilmTilePixel Declarations
	struct FilmTilePixel {
		Vector3 contribSum;
	};

	class FilmTile;

	// Film Declarations
	class Film {
	public:
		// Film Public Methods
		Film(const Point2i &resolution, const Bounds2f &cropWindow,
			const std::string &filename, Float scale,
			Float maxSampleLuminance = Infinity);
		Bounds2i GetSampleBounds() const;
		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i &sampleBounds);
		void MergeFilmTile(std::unique_ptr<FilmTile> tile);
		void WriteImage(Float splatScale = 1)
		{ }
		void Clear();

		// Film Public Data
		const Point2i fullResolution;
		const std::string filename;
		Bounds2i croppedPixelBounds;

	private:
		// Film Private Data
		struct Pixel {
			Pixel() { xyz[0] = xyz[1] = xyz[2] = 0; }
			Float xyz[3];
			Float pad;
		};
		std::unique_ptr<Pixel[]> pixels;
		std::mutex mutex;
		const Float scale;
		const Float maxSampleLuminance;

		// Film Private Methods
		Pixel &GetPixel(const Point2i &p) {
			CHECK(InsideExclusive(p, croppedPixelBounds));
			int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
			int offset = (p.x - croppedPixelBounds.pMin.x) + (p.y - croppedPixelBounds.pMin.y) * width;
			return pixels[offset];
		}
	};

	class FilmTile {
	public:
		// FilmTile Public Methods
		FilmTile(const Bounds2i &pixelBounds, Float maxSampleLuminance)
			: pixelBounds(pixelBounds),
			maxSampleLuminance(maxSampleLuminance) {
			pixels = std::vector<FilmTilePixel>(std::max(0, pixelBounds.Area()));
		}
		void AddSample(const Point2i &p, Vector3 L, Float sampleWeight = 1.) {
			FilmTilePixel &pixel = GetPixel(p);
			pixel.contribSum += L;
		}
		FilmTilePixel &GetPixel(const Point2i &p) {
			CHECK(InsideExclusive(p, pixelBounds));
			int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
			int offset =
				(p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
			return pixels[offset];
		}
		const FilmTilePixel &GetPixel(const Point2i &p) const {
			CHECK(InsideExclusive(p, pixelBounds));
			int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
			int offset =
				(p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
			return pixels[offset];
		}
		Bounds2i GetPixelBounds() const { return pixelBounds; }

	private:
		// FilmTile Private Data
		const Bounds2i pixelBounds;
		std::vector<FilmTilePixel> pixels;
		const Float maxSampleLuminance;
		friend class Film;
	};

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

		Vector2i filmSize() const
		{
			return Vector2i(_w, _h);
		}

		Bounds2i sampleBound() const
		{
			return Bounds2i(Point2i(0,0), Point2i(_w, _h));
		}

		Float aspect() const { return double(_w) / double(_h); };

		//! get a random ray based on (u, v) in image plane
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr);
		
	};

}

#endif