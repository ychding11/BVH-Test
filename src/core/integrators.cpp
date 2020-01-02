#include "mei.h"

#include "integrators.h"

#include "parallel.h"
#include "samplers.h"
#include "camera.h"
#include "scene.h"

namespace mei
{
    static int64_t nCameraRays;

	void SamplerIntegrator::Render(const Scene &scene)
	{
		// Compute number of tiles, _nTiles_, to use for parallel rendering
		Bounds2i sampleBounds = camera->sampleBound();
		Vector2i sampleExtent = camera->filmSize();
		const int tileSize = 16;
		Point2i nTiles((sampleExtent.x + tileSize - 1) / tileSize,
					   (sampleExtent.y + tileSize - 1) / tileSize);


		//ProgressReporter reporter(nTiles.x * nTiles.y, "Rendering");

		{
			// Render section of image corresponding to _tile_
			ParallelFor2D([&](Point2i tile) {

				// Get sampler instance for tile
				int seed = tile.y * nTiles.x + tile.x; //< random seed for current tile
				std::unique_ptr<Sampler> tileSampler = sampler->Clone(seed);

				// Compute sample bounds for current tile
				int x0 = sampleBounds.pMin.x + tile.x * tileSize;
				int x1 = std::min(x0 + tileSize, sampleBounds.pMax.x);
				int y0 = sampleBounds.pMin.y + tile.y * tileSize;
				int y1 = std::min(y0 + tileSize, sampleBounds.pMax.y);
				Bounds2i tileBounds(Point2i(x0, y0), Point2i(x1, y1));

				std::unique_ptr<FilmTile> filmTile = camera->pFilm->GetFilmTile(tileBounds);

				// Loop over pixels in tile to render them
				for (Point2i pixel : tileBounds)
				{
					tileSampler->StartPixel(pixel);

					// Do this check after the StartPixel() call; this keeps
				    // the usage of RNG values from (most) Samplers that use
				    // RNGs consistent, which improves reproducability /
				    // debugging.
					if (!InsideExclusive(pixel, pixelBounds))
						continue;

                    do {

                        Ray ray;
                        CameraSample cameraSample = tileSampler->GetCameraSample(pixel);
                        Float rayWeight = camera->GenerateRay(cameraSample, &ray);
                        nCameraRays++;

					    Vector3f L = Li(ray, scene, *tileSampler);

					    // Add camera ray's contribution to image
					    filmTile->AddSample(pixel, L);
                    } while (tileSampler->StartNextSample());
				}

				// Merge image tile into _Film_
				camera->pFilm->MergeFilmTile(std::move(filmTile));

				//	reporter.update();
			}, nTiles);
			//	reporter.done();
		}
	} // end render

}//< namespace

#include "interactions.h"

namespace mei
{

		NaiveIntegrator::NaiveIntegrator(bool cosSample, int ns,
			std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i &pixelBounds)
		: SamplerIntegrator(camera, sampler, pixelBounds),
		cosSample(cosSample)
		{
			nSamples = sampler->RoundCount(ns);
#if 0
			if (ns != nSamples)
				("Taking %d samples, not %d as specified", nSamples, ns);
#endif
			sampler->Request2DArray(nSamples);
	    }

	Vector3f NaiveIntegrator::Li(const Ray &r, const Scene &scene, Sampler &sampler, int depth) const
	{
		Ray ray(r);
		SurfaceInteraction isect;

		if (!scene.Intersect(ray, &isect))
		{
			return Vector3f(0,0,0);
		}
	    return Li(isect.SpawnRay(ray.d), scene, sampler, depth);
	}



    NaiveIntegrator *CreateNaiveIntegrator(std::shared_ptr<Sampler> sampler,
        std::shared_ptr<const Camera> camera) 
    {
        int maxDepth = 5;
        int pb[] = {0 };
        Bounds2i pixelBounds = camera->pFilm->GetSampleBounds();
        {
            pixelBounds = Intersect(pixelBounds, Bounds2i{ { pb[0], pb[2] },{ pb[1], pb[3] } });
            if (pixelBounds.Area() == 0)
                LOG(FATAL) << ("Degenerate \"pixelbounds\" specified.");
        }
        return new NaiveIntegrator(false, maxDepth, camera, sampler, pixelBounds);
    }

}//< namespace
