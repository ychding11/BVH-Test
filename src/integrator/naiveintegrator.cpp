#include "mei.h"

#include "naiveintegrator.h"

#include "parallel.h"
#include "scene.h"
#include "random/samplers.h"
#include "camera/camera.h"
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
			return Vector3f(0, 0, 0);
		}
		else
			return Vector3f(0.5, 0.5, 0.5);
	    //return Li(isect.SpawnRay(ray.d), scene, sampler, depth);
	}



    NaiveIntegrator *CreateNaiveIntegrator(std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera) 
    {
        int maxDepth = 5;
        Bounds2i pixelBounds = camera->pFilm->GetSampleBounds();
        if (pixelBounds.Area() == 0)
            LOG(ERROR) << ("Degenerate \"pixelbounds\" specified.");
        
        return new NaiveIntegrator(false, maxDepth, camera, sampler, pixelBounds);
    }

}//< namespace
