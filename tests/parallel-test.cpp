
#include "tests/gtest/gtest.h"
#include "parallel.h"
#include <atomic>

using namespace smallpt;

TEST(Parallel, Basics) {
    ParallelInit();

    std::atomic<int> counter{0};
    ParallelFor([&](int64_t) { ++counter; }, 1000, 1);
    EXPECT_EQ(1000, counter);

    counter = 0;
    ParallelFor([&](int64_t) { ++counter; }, 1000, 19);
    EXPECT_EQ(1000, counter);

#if 0
    counter = 0;
    ParallelFor2D([&](Point2i p) { ++counter; }, Point2i(15, 14));
    EXPECT_EQ(15*14, counter);
#endif

    ParallelCleanup();
}

TEST(Parallel, DoNothing) {
    ParallelInit();

    std::atomic<int> counter{0};
    ParallelFor([&](int64_t) { ++counter; }, 0, 1);
    EXPECT_EQ(0, counter);
#if 0
    counter = 0;
    ParallelFor2D([&](Point2i p) { ++counter; }, Point2i(0, 0));
    EXPECT_EQ(0, counter);
#endif

    ParallelCleanup();
}
