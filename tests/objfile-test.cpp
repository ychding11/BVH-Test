
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

#include <memory>

using namespace mei;

TEST(objFile, Basic)
{
	std::string	objFileName{ "../data/bunny.obj" };
	ObjFile objFile;
	EXPECT_EQ(true, objParseFile(objFile, objFileName.c_str()));
	EXPECT_EQ(true, objValidate(objFile));

	int nTriangles;

	//CreateTriangleMeshShape();
}
