
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "primitives.h"
#include "scene.h"
#include "shapes.h"
#include "camera/camera.h"
//#include "random/samplers.h"
#include "random/naivesampler.h"
#include "integrator/naiveintegrator.h"

#include <memory>

using namespace mei;

//< Create a quad for test
static std::shared_ptr<Primitive> CreateQuad()
{
	//< hard coded data
    int nTriangles = 2;
    int nVertices = 4;
    int vertexIndices[] = {0, 1, 2, 0, 2, 3};
    const Point3f  p[] = { {-0.5, 0.5, 0}, {-0.5, -0.5, 0}, {0.5, -0.5, 0}, {0.5, 0.5, 0}};
    const Normal3f n[] = { {0, 0, 1}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}};

	//< triangle mesh Shape
    std::vector<std::shared_ptr<Shape>> shapes;
    shapes = CreateTriangleMeshShape( nTriangles, vertexIndices, nVertices, p, n);

	//< geometric primitive
    std::vector<std::shared_ptr<Primitive>> prims;
    prims.reserve(shapes.size());
    for (auto s : shapes)
    {
        prims.push_back( std::make_shared<GeometricPrimitive>(s));
    }

	//< aggregate primitive
    std::shared_ptr<Primitive> aggregate;
    aggregate = CreateListAggregate(prims);

    return aggregate;
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
