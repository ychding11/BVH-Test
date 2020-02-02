
#include "camera.h"
#include <fstream>

namespace mei
{

	// Film Method Definitions
	Film::Film(const Point2i &resolution, const Bounds2f &cropWindow,
		const std::string &filename, Float scale, Float maxSampleLuminance)
		: fullResolution(resolution), filename(filename), scale(scale), maxSampleLuminance(maxSampleLuminance)
	{
		// Compute film image bounds
		croppedPixelBounds =
			Bounds2i(Point2i(std::ceil(fullResolution.x * cropWindow.pMin.x), std::ceil(fullResolution.y * cropWindow.pMin.y)),
					 Point2i(std::ceil(fullResolution.x * cropWindow.pMax.x), std::ceil(fullResolution.y * cropWindow.pMax.y)));
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

    static inline float clamp(Float x) { return x < 0.0 ? 0.0 : x > 1.0 ? 1.0 : x; }

    //< convert RGB float in range [0.0, 1.0] to [0, 255] and perform gamma correction
    static inline int toInt(Float x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

    //< Write image to PPM file, a very simple image file format
    //< Use C-Style IO
    void Film::savePPM() const
    {
        FILE *f = fopen(filename.c_str(), "w");
        fprintf(f, "P3\n%d %d\n%d\n", fullResolution.x, fullResolution.y, 255);

        for (int i = 0; i < fullResolution.x * fullResolution.y; i++)  // loop over pixels, write RGB values
        {
            const Pixel &pixel = pixels[i];

            fprintf(f, "%d %d %d ",
                toInt(pixel.xyz[0]),
                toInt(pixel.xyz[1]),
                toInt(pixel.xyz[2]));
        }

        fclose(f);
    }

    void Film::WriteImage(Float /* splatScale */) const
    {
        savePPM();
    }

    void FilmTester::DrawCircle()
    {
        //< determine center of a circle
        Point2i c = film->croppedPixelBounds.Center();

        //< determine radius of a circle
        Vector2i diag = film->croppedPixelBounds.Diagonal();
        int r = diag[film->croppedPixelBounds.MaximumExtent()] / 2;

        //< loop over all pixels to test
        //< inside a circle or not
        for (Point2i p : film->croppedPixelBounds)
        {
            Film::Pixel &pixel = film->GetPixel(p);
            if (Distance(p, c) <= r)
            {
                pixel.xyz[0] = 0.5;
                pixel.xyz[1] = 0.5;
                pixel.xyz[2] = 0.5;
            }
            else
            {
                pixel.xyz[0] = 0;
                pixel.xyz[1] = 0;
                pixel.xyz[2] = 0;
            }
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



	Film *CreateFilm(int width, int height)
	{
		std::string filename = "test.ppm";

		int xres = width;
		int yres = height;
        Bounds2f crop{ {0., 0.}, {1., 1.} };
        Float scale = 1.; 
        Float maxSampleLuminance = Infinity;
		return new Film(Point2i(xres, yres), crop,  filename, scale, maxSampleLuminance);
	}

	Camera *CreateCamera( Film *film)
	{
		Float halffov = 45;
		Float fov = 2.f * halffov;
		Point3f  position{0,0,1};
		Vector3f dir{0,0,-1};
		return new Camera(position,dir,fov, film);
	}

	//Camera *CreateCamera(Film *film, Point3f position={0,0,1}, Point3f target={0,0,0}, Float halfVFov=45)
	Camera *CreateCamera(Film *film, Point3f position, Point3f target, Float halfVFov)
	{
		Float fov = 2.f * halfVFov;
		Vector3f dir = Normalize(target-position);
		return new Camera(position,dir,fov, film);
	}
}