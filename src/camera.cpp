
#include "camera.h"

namespace mei
{

	// Film Method Definitions
	Film::Film(const Point2i &resolution, const Bounds2f &cropWindow,
		const std::string &filename, Float scale, Float maxSampleLuminance)
		: fullResolution(resolution), filename(filename), scale(scale), maxSampleLuminance(maxSampleLuminance)
	{
		// Compute film image bounds
		croppedPixelBounds =
			Bounds2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x),
							 std::ceil(fullResolution.y * cropWindow.pMin.y)),
							 Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x),
					std::ceil(fullResolution.y * cropWindow.pMax.y)));
		LOG(INFO) << "Created film with full resolution " << resolution.str() <<
			". Crop window of " << cropWindow.str() << " -> croppedPixelBounds " << croppedPixelBounds.str();

		// Allocate film image storage
		pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBounds.Area()]);
		//filmPixelMemory += croppedPixelBounds.Area() * sizeof(Pixel);
	}

	Bounds2i Film::GetSampleBounds() const
	{
		return croppedPixelBounds;
	}

	std::unique_ptr<FilmTile> Film::GetFilmTile(const Bounds2i &sampleBounds)
	{
		return std::unique_ptr<FilmTile>(new FilmTile(sampleBounds, maxSampleLuminance));
	}

	void Film::Clear()
	{
		for (Point2i p : croppedPixelBounds)
		{
			Pixel &pixel = GetPixel(p);
			pixel.xyz[0] = 0;
			pixel.xyz[1] = 0;
			pixel.xyz[2] = 0;
		}
	}

	void Film::MergeFilmTile(std::unique_ptr<FilmTile> tile)
	{
		//VLOG(1) << "Merging film tile " << tile->pixelBounds.str();
		//std::lock_guard<std::mutex> lock(mutex);
		for (Point2i pixel : tile->GetPixelBounds())
		{
			const FilmTilePixel &tilePixel = tile->GetPixel(pixel);
			Pixel &mergePixel = GetPixel(pixel);
			Vector3f rgb = tilePixel.contribSum;
			//for (int i = 0; i < 3; ++i) mergePixel.xyz[i] += rgb[i];
			mergePixel.xyz[0] += rgb[0];
			mergePixel.xyz[1] += rgb[1];
			mergePixel.xyz[2] += rgb[2];
		}
	}

	/*******************************************************************************
    *  coordinate is tightly coupled with width and height of image
	********************************************************************************/
	void Camera::constructCoordinate()
	{
		Float halfVfov = _vfov * 0.5 * (M_PI / 180.);
        Float h = std::tanf(halfVfov);
		Float aspect = double(_w) / double(_h);
		_u = Vector3f(aspect * h, 0, 0);
		_v = Normalize(Cross(_u, _d)) * h;
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
        Vector3f d = _u * x + _v * y + _d;
		//Ray r(_p + d * 140, d.norm());
		Ray r(_p, Normalize(d));
		return r;
	}

}