
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "primitives.h"
#include "scene.h"
#include "camera/camera.h"
//#include "random/samplers.h"
#include "random/naivesampler.h"
#include "integrator/naiveintegrator.h"

#include <memory>

using namespace mei;

//< Create a quad for test
static std::shared_ptr<Primitive> CreateQuad()
{
    return nullptr;
}

TEST(Camera, Basics)
{
    int width = 4;
    int height = 4;
	std::unique_ptr<Film> pfilm(CreateFilm(width, height));

	const Point2i ResolutionExpected{ width, height};
	EXPECT_EQ(ResolutionExpected, pfilm->fullResolution);

	EXPECT_EQ("test.png", pfilm->filename);

	const Bounds2f cropExpected{ {0, 0}, {1, 1} };
    const Bounds2i cropResolutionExpected{
        Point2i{(int)std::ceil(ResolutionExpected.x * cropExpected.pMin.x), (int)std::ceil(ResolutionExpected.y * cropExpected.pMin.y)},
        Point2i{(int)std::ceil(ResolutionExpected.x * cropExpected.pMax.x), (int)std::ceil(ResolutionExpected.y * cropExpected.pMax.y)}
    };
	EXPECT_EQ(cropResolutionExpected, pfilm->croppedPixelBounds);

    std::shared_ptr<Camera> camera(CreateCamera(pfilm.get()));
    std::shared_ptr<Sampler> sampler(CreateNaiveSampler(4));
    //CreateNaiveIntegrator

    std::unique_ptr<Scene> scene(new Scene(CreateQuad()));
    std::unique_ptr<Integrator> integrator(CreateNaiveIntegrator(sampler, camera));

    for (auto p : cropResolutionExpected)
    {
        LOG(ERROR) << "Begin test pixel:" << p.str();
        Ray ray;
        CameraSample cameraSample = sampler->GetCameraSample(p);
        camera->GenerateRay(cameraSample, &ray);
    }



}
