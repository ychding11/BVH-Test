#ifndef NAIVE_INTEGRATOR_H_ 
#define NAIVE_INTEGRATOR_H_ 

#include "2d.h"
#include "3d.h"
#include "integrator/integrators.h"

namespace mei
{

	class Material;
	class Scene;
    class Sampler;
    class Camera;

	//< define a "test integrator" to test algorithms
	class NaiveIntegrator : public SamplerIntegrator
	{
	public:
		// AOIntegrator Public Methods
		NaiveIntegrator(bool cosSample, int nSamples,
			std::shared_ptr<const Camera> camera,
			std::shared_ptr<Sampler> sampler,
			const Bounds2i &pixelBounds);

		virtual Vector3f Li(const Ray &ray, const Scene &scene, Sampler &sampler, int depth) const override;
	private:
		bool cosSample;
		int  nSamples;
    };

    NaiveIntegrator *CreateNaiveIntegrator(std::shared_ptr<Sampler> sampler, std::shared_ptr<const Camera> camera);

} //namespace

#endif
