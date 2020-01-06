

#ifndef API_H_ 
#define API_H_ 

#include "2d.h"
#include "3d.h"
#include "paramset.h"
#include <map>

namespace mei
{
    class Integrator;
    class Scene;
    class Camera;
    class Primitive;
    class Light;

    struct RenderContext
    {
        Integrator *MakeIntegrator() const;
        Scene *MakeScene();
        Camera *MakeCamera() const;

        //Float transformStartTime = 0, transformEndTime = 1;
        std::string FilterName = "box";
        ParamSet FilterParams;
        std::string FilmName = "image";
        ParamSet FilmParams;
        std::string SamplerName = "halton";
        ParamSet SamplerParams;
        std::string AcceleratorName = "bvh";
        ParamSet AcceleratorParams;
        std::string IntegratorName = "path";
        ParamSet IntegratorParams;
        std::string CameraName = "perspective";
        ParamSet CameraParams;
        //TransformSet CameraToWorld;
        //std::map<std::string, std::shared_ptr<Medium>> namedMedia;
        std::vector<std::shared_ptr<Light>> lights;
        std::vector<std::shared_ptr<Primitive>> primitives;
        //std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
        std::vector<std::shared_ptr<Primitive>> *currentInstance = nullptr;
        bool haveScatteringMedia = false;
    };

	static std::unique_ptr<RenderContext> renderContext;


	// API Function Definitions
	void meiInit();
	void meiCleanup();

}
 //namespace

#endif
