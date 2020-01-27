
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "primitives.h"
#include "scene.h"
#include "shapes.h"
#include "camera/camera.h"
#include "parallel.h"
//#include "random/samplers.h"
#include "random/naivesampler.h"
#include "integrator/naiveintegrator.h"

#include <memory>

using namespace mei;

class Drawer 
{

public:
	Drawer( std::shared_ptr<Sampler> sampler, std::shared_ptr<Film> film)
		: sampler(sampler)
		, pFilm(film)
		, pixelBounds(pFilm->GetSampleBounds())
		, sampleExtent(pFilm->fullResolution)
	{ }

	virtual Vector3f PixelValue(Point2i pixel, Sampler &sampler) const = 0;

	void Draw() ;

private:
	std::shared_ptr<Sampler> sampler;
	std::shared_ptr<Film> pFilm;
	const Bounds2i pixelBounds;
	const Vector2i sampleExtent;
};

void Drawer::Draw()
{
	const int tileSize = 16;
	Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
		           (sampleExtent.y + tileSize - 1) / tileSize);

	//ProgressReporter reporter(nTiles.x * nTiles.y, "Rendering");

	{
		// Render section of image corresponding to _tile_
		ParallelFor2D([&](Point2i tile) {
			int seed = tile.y * nTiles.x + tile.x; //< random seed for current tile
			std::unique_ptr<Sampler> tileSampler = sampler->Clone(seed);

			// Compute pixel bounds for current tile
			int x0 = pixelBounds.pMin.x + tile.x * tileSize;
			int x1 = std::min(x0 + tileSize, pixelBounds.pMax.x);
			int y0 = pixelBounds.pMin.y + tile.y * tileSize;
			int y1 = std::min(y0 + tileSize, pixelBounds.pMax.y);
			Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

			std::unique_ptr<FilmTile> filmTile = pFilm->GetFilmTile(tileBounds);

			// Loop over pixels in tile to render them
			for (Point2i pixel : tileBounds)
			{
				tileSampler->StartPixel(pixel);
				if (!InsideExclusive(pixel, pixelBounds))
					continue;

				do {
					Vector3f L = PixelValue(pixel, *tileSampler);

					// Add camera ray's contribution to image
					filmTile->AddSample(pixel, L);
				} while (tileSampler->StartNextSample());
			}

			// Merge image tile into _Film_
			pFilm->MergeFilmTile(std::move(filmTile));

			//	reporter.update();
		}, nTiles);
		//	reporter.done();
	}

    pFilm->WriteImage();
}

class Circle :public Drawer
{
public:

	Circle(std::shared_ptr<Sampler> sampler, std::shared_ptr<Film> film)
		: Drawer(sampler, film)
	{
	}

	virtual Vector3f PixelValue(Point2i pixel, Sampler &sampler) const override;
};

Vector3f Circle::PixelValue(Point2i pixel, Sampler &sampler) const
{
	Vector3f v(1,0,0);

	return v;
}

TEST(Film, FilmTest)
{
    int width  = 128;
    int height = 128;
	std::shared_ptr<Film> pfilm(CreateFilm(width, height));
	const Point2i ResolutionExpected{ width, height};
	EXPECT_EQ(ResolutionExpected, pfilm->fullResolution);
	EXPECT_EQ("test.ppm", pfilm->filename);

	std::shared_ptr<Sampler> sampler{ CreateNaiveSampler() };

    Circle circle{ sampler,pfilm };
	circle.Draw();
	system("ffplay.exe test.ppm");
}
