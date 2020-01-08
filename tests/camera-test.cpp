
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "2d.h"
#include "3d.h"
#include "camera/camera.h"

#include <memory>

using namespace mei;

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
    for (auto p : cropResolutionExpected)
    {
        LOG(INFO) << p.str();
    }

    std::unique_ptr<Camera> camera(CreateCamera(pfilm.get()));


}
