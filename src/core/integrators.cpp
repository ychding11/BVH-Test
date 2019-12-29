
#include "integrators.h"
#include "parallel.h"
#include "interactions.h"

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


		MyIntegrator::MyIntegrator(bool cosSample, int ns,
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

	Vector3f MyIntegrator::Li(const Ray &r, const Scene &scene, Sampler &sampler, int depth) const
	{
		Vector3f L(0,0,0);
		Ray ray(r);
		SurfaceInteraction isect;

		if (!scene.Intersect(ray, &isect))
		{
			return Vector3f(0,0,0);
		}

		const Object &obj = *hitInfo.object;        // the hit object
		Vector3 x = hitInfo.hit, n = hitInfo.object->getNormal(hitInfo);
		Vector3 nl = n.dot(r.d) < 0 ? n : n * -1;
		Vector3 f = obj.c;
		const Float p = .1; //< RR stop Pr

		if (++depth > 4 && depth <= 6)
		{
			if (erand48(Xi) < p) return obj.e;
			else f = f * (1 / (1. - p));
		}
		else if (depth > 6)
		{
			return obj.e;
		}

		if (obj.refl == DIFF) // Ideal DIFFUSE reflection
		{
			// Why converge slow with spp goes up ?
			// So how to let diffuse converges fast ?
			return obj.e + f.cmult(myradiance(Ray(x, cosWeightedSample(nl, Xi)), depth, Xi));
		}
		else if (obj.refl == SPEC) // Ideal SPECULAR reflection
		{
			return obj.e + f.cmult(myradiance(Ray(x, reflect(r.d, n)), depth, Xi));
		}
		else // Ideal dielectric REFRACTION
		{
			//! Maybe write a single function to do this things.
			//! Input: IOR1, IOR2, incomming direction(source->shading point), normal
			Ray reflRay(x, reflect(r.d, n));
			bool into = n.dot(nl) > 0;     // Ray from outside going in
			Float nc = 1.;   // Air
			Float nt = 1.5;  // IOR  Glass
			Float nnt = into ? nc / nt : nt / nc,
				ddn = r.d.dot(nl),
				cos2t;
			if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) < 0)    // Total internal reflection
			{
				return obj.e + f.cmult(myradiance(reflRay, depth, Xi));
			}
			Vector3 tdir = (r.d*nnt - n * ((into ? 1 : -1) * (ddn*nnt + sqrt(cos2t)))).norm();

			Float Re = SchlickApproxim(nt, nc, into ? -ddn : tdir.dot(n)); //! Schlick's approximation
			Float Tr = 1 - Re;

			// Russian roulette weight
			Float P = .25 + .5 * Re,
				RP = Re / P,
				TP = Tr / (1 - P);
			return obj.e + f.cmult(depth > 2 ? (erand48(Xi) < P ?   // Russian roulette
				myradiance(reflRay, depth, Xi)*RP : myradiance(Ray(x, tdir), depth, Xi)*TP) :
				myradiance(reflRay, depth, Xi)*Re + myradiance(Ray(x, tdir), depth, Xi)*Tr);
		}
		return L;
	}

}//< namespace
