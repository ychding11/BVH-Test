#ifndef PARALLEL_H_
#define PARALLEL_H_

#include <functional>

namespace smallpt
{


// Called in Main Thread
void parallelInit(void);

// Called in Main Thread
void parallelCleanup(void);

// PUBLIC interface
// Called by Main Thread
// count: total work in a ParallelForLoop
// chunkSize:  work for a single worker in a ParallelLoop
void ParallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize);

// Called in Main Thread
// Main Thread would kick off all worker to merge statistics.
// At this time, all worker thread should wait on an empty work list.
// Main Thread would merge its own work independently.
void MergeWorkerThreadStats();

}

#endif