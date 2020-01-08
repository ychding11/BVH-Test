#ifndef CAMERA_H
#define CAMERA_H

#include <math.h>
#include <stdlib.h>
#include <mutex>
#include <iostream>
#include <sstream>

#include "2d.h"
#include "3d.h"
#include "random/randoms.h"

namespace mei
{
    struct CameraSample
    {
        Point2f pFilm;
        Point2f pLens;
        Float time;
    };

	struct FilmTilePixel
	{
		Vector3f contribSum;
	};

    //< Forward declare
	class FilmTile;

	class Film
	{
	public:
		// Film Public Methods
		Film(const Point2i &resolution, const Bounds2f &cropWindow,
			const std::string &filename, Float scale,
			Float maxSampleLuminance = Infinity);

		Bounds2i GetSampleBounds() const;

		std::unique_ptr<FilmTile> GetFilmTile(const Bounds2i &sampleBounds);

		void MergeFilmTile(std::unique_ptr<FilmTile> tile);

		void WriteImage(Float splatScale = 1) { }

		void Clear();

		// Film Public Data
		const Point2i fullResolution;
		const std::string filename;
		Bounds2i croppedPixelBounds;

	private:
		// Film Private Data
		struct Pixel
		{
			Pixel() { xyz[0] = xyz[1] = xyz[2] = 0; }
			Float xyz[3];
			Float pad;
		};
		std::unique_ptr<Pixel[]> pixels;
		std::mutex mutex;
		const Float scale;
		const Float maxSampleLuminance;

		// Film Private Methods
		Pixel &GetPixel(const Point2i &p)
		{
			CHECK(InsideExclusive(p, croppedPixelBounds));
			int width = croppedPixelBounds.pMax.x - croppedPixelBounds.pMin.x;
			int offset = (p.x - croppedPixelBounds.pMin.x) + (p.y - croppedPixelBounds.pMin.y) * width;
			return pixels[offset];
		}
	};

	class FilmTile
	{
	public:
		// FilmTile Public Methods
		FilmTile(const Bounds2i &pixelBounds, Float maxSampleLuminance)
			: pixelBounds(pixelBounds), maxSampleLuminance(maxSampleLuminance)
		{
			pixels = std::vector<FilmTilePixel>(std::max(0, pixelBounds.Area()));
		}
		void AddSample(const Point2i &p, Vector3f L, Float sampleWeight = 1.)
		{
			FilmTilePixel &pixel = GetPixel(p);
			pixel.contribSum += L;
		}
		FilmTilePixel& GetPixel(const Point2i &p)
		{
			CHECK(InsideExclusive(p, pixelBounds));
			int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
			int offset = (p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
			return pixels[offset];
		}
		const FilmTilePixel& GetPixel(const Point2i &p) const
		{
			CHECK(InsideExclusive(p, pixelBounds));
			int width = pixelBounds.pMax.x - pixelBounds.pMin.x;
			int offset = (p.x - pixelBounds.pMin.x) + (p.y - pixelBounds.pMin.y) * width;
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
	protected:
		Float _vfov;
		uint32_t _w, _h;
		Point3f  _p; //< coordiate & position
		Vector3f _u, _v, _d; //< coordiate & position

		std::ostringstream _ss;

		/*******************************************************************************
		*  coordinate is tightly coupled with width and height of image
		********************************************************************************/
		void constructCoordinate();
		
        //! get a sample in [0,0), support subpixel;
		Float getSample(unsigned short *X = nullptr);
        
	public:
        Film *pFilm;

		Camera() = delete; //< No default constructor allowed
		//Camera(uint32_t w, uint32_t _h, Float vfov);

		//! \param position is in world space
		//! \param dir is in world space and is normalized
		Camera(Point3f position, Vector3f dir, Float vfov, Film *film)
			: _p(position), _d(dir)
			, _w(0), _h(0)
			, _vfov(vfov)
			, pFilm(film)
		{
			_w = film->fullResolution.x;
			_h = film->fullResolution.y;
			constructCoordinate();
		}

		Vector2i FilmSize() const
		{
			return Vector2i(_w, _h);
		}

		void FilmSize(uint32_t w, uint32_t h)
		{
			if (w != _w || h != _h)
			{
				_w = w; _h = h;
				constructCoordinate();
			}
		}

		Bounds2i SampleBound() const
		{
			return Bounds2i(Point2i(0,0), Point2i(_w, _h));
		}

		Float Aspect() const { return double(_w) / double(_h); };

		//! get a random ray based on (u, v) in image plane
		Ray getRay(uint32_t u, uint32_t v, unsigned short *X = nullptr);

        Float GenerateRay(const CameraSample &sample, Ray *ray) const
        {
			Float xOffset = _w / 2.;
			Float yOffset = _h / 2.;

            // adjust original point 
            Point3f pFilm = Point3f(sample.pFilm.x-xOffset, sample.pFilm.y-yOffset, 0);
			Vector3f d = _u * pFilm.x + _v * pFilm.y + _d;
            *ray = Ray(_p, Normalize(d));
            return 1;
        }
	};

	Film   *CreateFilm(int width = 1280, int height = 720);
	Camera *CreateCamera(Film *film);

} //<namespace
#endif
