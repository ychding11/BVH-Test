
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "primitives.h"
#include "scene.h"
#include "shapes.h"
#include "camera/camera.h"
#include "parallel.h"
//#include "random/samplers.h"
#include "random/naivesampler.h"
#include "integrator/naiveintegrator.h"
#include "utils/objparser.h"
#include "utils/modelimporter.h"

#include <memory>

using namespace mei;

TEST(objFile, Basic)
{
	std::string	objFileName{ "../data/bunny.obj" };
	ObjFile objFile;
	EXPECT_EQ(true, objParseFile(objFile, objFileName.c_str()));
	EXPECT_EQ(true, objValidate(objFile));
}

TEST(objFile, assimp)
{
	std::string	objFileName{ "../data/bunny.obj" };
	ModelImporter importer;
	importer.LoadModel(objFileName);
	EXPECT_EQ(true, importer.meshes.size() > 0);

	//< geometric primitive
	std::vector<std::shared_ptr<Shape>> &shapes = importer.meshes[0];
	std::vector<std::shared_ptr<Primitive>> prims;
	prims.reserve(shapes.size());
	for (auto s : shapes)
	{
		prims.push_back(std::make_shared<GeometricPrimitive>(s));
	}

	//< aggregate primitive
	std::shared_ptr<Primitive> aggregate;
	aggregate = CreateListAggregate(prims);

	Bounds3f bb= aggregate->WorldBound();

	//EXPECT_EQ(false, );

	Point3f target = aggregate->WorldBound().Center();
	Point3f position = target + Vector3f{0,0,1} * 5;

	int width  = 64;
	int height = 64;
	std::unique_ptr<Film> pfilm(CreateFilm(width, height));
	std::shared_ptr<Camera> camera(CreateCamera(pfilm.get(), position, target, 45));
	std::shared_ptr<Sampler> sampler(CreateNaiveSampler(4));

	std::unique_ptr<Scene> scene(new Scene(aggregate));
	std::unique_ptr<Integrator> integrator(CreateNaiveIntegrator(sampler, camera));

	ParallelInit();

	integrator->Render(*scene);

	ParallelCleanup();

	pfilm->WriteImage();
	system("ffplay.exe test.ppm");
}
