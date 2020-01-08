#ifndef INTEGRATOR_H_ 
#define INTEGRATOR_H_ 

#include "2d.h"
#include "3d.h"


namespace mei
{

	class Material;
	class Scene;
    class Sampler;
    class Camera;

	//< It determines how to render
	class Integrator
	{
	public:
		virtual void Render(const Scene &scene) = 0;
	};

	class SamplerIntegrator:public Integrator
	{

	public:
        // SamplerIntegrator Public Methods
        SamplerIntegrator(std::shared_ptr<const Camera> camera,
            std::shared_ptr<Sampler> sampler,
            const Bounds2i &pixelBounds)
            : camera(camera), sampler(sampler), pixelBounds(pixelBounds) {}

        virtual void Preprocess(const Scene &scene, Sampler &sampler) {}

        virtual Vector3f Li(const Ray&ray, const Scene &scene, Sampler &sampler, int depth = 0) const = 0;

		virtual void Render(const Scene &scene) override;

    protected:
        // SamplerIntegrator Protected Data
        std::shared_ptr<const Camera> camera;

    private:
        // SamplerIntegrator Private Data
        std::shared_ptr<Sampler> sampler;
        const Bounds2i pixelBounds;
	};


#if 0
    //! caculate reflected ray direction. dot(in, n) < 0
    //! in is unit vector, n is unit vector, result is unit vector
	inline Vector3f reflect(const Vector3f &in, const Vector3f &n)
	{
		return in - n * 2 * Dot(n,in);
	}

    //! Schlick's approximation:
    //!  https://en.wikipedia.org/wiki/Schlick%27s_approximation
    //!
    inline Float SchlickApproxim(Float n1, Float n2, Float cosTheta)
    {
		Float a = n1 - n2, b = n1 + n2, R0 = (a * a) / (b*b),
			  c = 1 - cosTheta;  //! Term: 1 - cos(theta)
		Float Re = R0 + (1 - R0)*c*c*c*c*c; // Specular Relection & Transmission
        return Re;
    }

    //! put an attention to normal n
    //! consine weighed hemisphere sample
    inline Vector3f cosWeightedSample(const Vector3f &n, unsigned short *Xi = nullptr)
    {
        Float r1 , r2 ;
        if (Xi)
        {
            r1 = 2 * M_PI * erand48(Xi); r2 = erand48(Xi);
        }
        else
        {
            r1 = 2 * M_PI * randomFloat(); r2 = randomFloat();
        }
        Float r2s = sqrt(r2);
        Vector3f w = n, u = Normalize(Cross((fabs(w.x) > .1 ? Vector3f(0, 1, 0) : Vector3f(1,0,0)) , w)), v = Cross(w, u);

        Vector3f d = (u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2));
        return Normalize(d);
    }
#endif

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
