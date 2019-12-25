#ifndef INTEGRATOR_H_ 
#define INTEGRATOR_H_ 

#include "3d.h"

namespace mei
{

	class Material;
	class Scene;
	//< It determines how to render
	class Integrator
	{
	public:
		virtual void Render(const Scene &scene) = 0;
	};

	class SampleIntegrator:public Integrator
	{

	public:
		virtual void Render(const Scene &scene) override;
	};

}

#endif
