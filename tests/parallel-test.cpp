
#include "tests/gtest/gtest.h"
#include "mei.h"
#include "parallel.h"
#include <atomic>

using namespace mei;

TEST(Parallel, Basics) {
    ParallelInit();

    std::atomic<int> counter{0};
    ParallelFor([&](int64_t) { ++counter; }, 1000, 1);
    EXPECT_EQ(1000, counter);

    counter = 0;
    ParallelFor([&](int64_t) { ++counter; }, 1000, 19);
    EXPECT_EQ(1000, counter);

    counter = 0;
    ParallelFor2D([&](Point2i p) { ++counter; }, Point2i(5, 4));
    EXPECT_EQ(5*4, counter);

    ParallelCleanup();
}

TEST(Parallel, DoNothing) {
    ParallelInit();

    std::atomic<int> counter{0};
    ParallelFor([&](int64_t) { ++counter; }, 0, 1);
    EXPECT_EQ(0, counter);

    counter = 0;
    ParallelFor2D([&](Point2i p) { ++counter; }, Point2i(0, 0));
    EXPECT_EQ(0, counter);

    ParallelCleanup();
}
