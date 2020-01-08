
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "camera/camera.h"

#include <memory>

using namespace mei;

TEST(Camera, Basics)
{
	std::unique_ptr<Film> pfilm(CreateFilm());

	const Point2i cropRes{ 1280, 720 };
	EXPECT_EQ(cropRes, pfilm->fullResolution);

	EXPECT_EQ("test.png", pfilm->filename);

	const Bounds2i cropExpected{ {0, 1}, {0, 1} };
	EXPECT_EQ(cropExpected, pfilm->croppedPixelBounds);
}
