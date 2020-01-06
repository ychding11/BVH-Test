#include "mei.h"

#include "api.h"

#include "stringprint.h"
#include "parallel.h"

#include "random/samplers.h"
#include "camera/camera.h"
#include "integrator/integrators.h"
#include "scene.h"

namespace mei
{

	Film *MakeFilm()
	{
		Film *film = CreateFilm();
		return film;
	}

	Camera *MakeCamera(Film *pfilm)
	{
		Camera *camera = CreateCamera(pfilm);
		return camera;
	}
	Camera *RenderContext::MakeCamera() const
	{
		Film *film = MakeFilm();
		if (!film)
		{
			LOG(ERROR) << ("Unable to create film.");
			return nullptr;
		}
		Camera *camera = mei::MakeCamera(film);
		return camera;
	}


    std::shared_ptr<Sampler> MakeSampler(const std::string &name, const ParamSet &paramSet, const Film *film)
    {
        Sampler *sampler = nullptr;
        if (name == "lowdiscrepancy" || name == "02sequence")
        {

        }
        else if (name == "maxmindist")
        {

        }
        else if (name == "halton")
        {

        }
        else if (name == "sobol")
        {

        }
        else if (name == "random")
        {

        }
        else if (name == "stratified")
        {

        }
        else
        {
            LOG(ERROR) << ("Sampler \"%s\" unknown.", name.c_str());
        }
        return std::shared_ptr<Sampler>(sampler);
    }

	Integrator *RenderContext::MakeIntegrator() const
	{
		std::shared_ptr<const Camera> camera(MakeCamera());
		if (!camera)
		{
			LOG(ERROR) << ("Unable to create camera");
			return nullptr;
		}

		std::shared_ptr<Sampler> sampler = MakeSampler(SamplerName, SamplerParams, camera->pFilm);
		if (!sampler)
		{
			LOG(ERROR) << ("Unable to create sampler.");
			return nullptr;
		}

		Integrator *integrator = nullptr;
		if (IntegratorName == "whitted")
		{
			LOG(ERROR) << ( "Not Supported yet.");
		}
		else if (IntegratorName == "directlighting")
		{
			LOG(ERROR) << ( "Not Supported yet.");
		}
		else if (IntegratorName == "naive")
		{
			integrator = CreateNaiveIntegrator(sampler, camera);
		}
		else
		{
			LOG(ERROR) << ("Integrator \"%s\" unknown.", IntegratorName.c_str());
			return nullptr;
		}

		// Warn if no light sources are defined
		if (lights.empty())
			LOG(ERROR) << ( "No light sources defined in scene; ");
		return integrator;
	}

	std::shared_ptr<Primitive> MakeAccelerator( const std::string &name, std::vector<std::shared_ptr<Primitive>> prims, const ParamSet &paramSet)
	{
		std::shared_ptr<Primitive> accel;
		if (name == "bvh")
		{
			LOG(ERROR) << StringPrintf("Accelerator \"%s\" Not supported yet.", name.c_str());
		}
		else if (name == "kdtree")
		{
			LOG(ERROR) << StringPrintf("Accelerator \"%s\" Not supported yet.", name.c_str());
		}
		else
		{
			LOG(ERROR) << StringPrintf("Accelerator \"%s\" unknown. Use naive one.", name.c_str());
		}
		return accel;
	}

	Scene *RenderContext::MakeScene()
	{
		std::shared_ptr<Primitive> accelerator = MakeAccelerator(AcceleratorName, std::move(primitives), AcceleratorParams);
		if (!accelerator)
		{
			LOG(ERROR) << ("Accelerator \"%s\" is null.");
            return nullptr;
		}
		Scene *scene = new Scene(accelerator);

		// Erase primitives and lights from _RenderOptions_
		primitives.clear();
		lights.clear();
		return scene;
	}


	void singleTriangleTest(void)
	{

	}

	void meiInit()
	{
		renderContext.reset(new RenderContext);
		ParallelInit();
	}


    void meiRun()
    {
        std::unique_ptr<Integrator> integrator(renderContext->MakeIntegrator());
        std::unique_ptr<Scene> scene(renderContext->MakeScene());
        if (scene && integrator) integrator->Render(*scene);
    }

	void meiCleanup()
	{
		ParallelCleanup();
	}

}//< namespace
