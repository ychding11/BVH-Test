#include <vector>
#include <iostream>
#include <cstdio>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <algorithm>

#include "parallel.h"
#include "stats.h"

#ifndef GLOG_NO_ABBREVIATED_SEVERITIES
#define GLOG_NO_ABBREVIATED_SEVERITIES
#endif

#include <glog/logging.h>

//format syntax  http://fmtlib.net/latest/syntax.html

namespace mei
{

	class ParallelForLoop; // forward declare.

	static bool shutdowThread = false;

	static std::vector<std::thread> threads;

	static std::mutex workListMutex;
	static std::condition_variable workListCondition;

	static ParallelForLoop *workList = nullptr;

	thread_local int threadIndex;

	// Bookkeeping variables to help with the implementation of
	// MergeWorkerThreadStats().
	static std::atomic<bool> reportWorkerStats{ false };

	// Number of workers that still need to report their stats.
	static std::atomic<int> reporterCount;

	// After kicking the workers to report their stats, the main thread waits
	// on this condition variable until they've all done so.
	static std::condition_variable reportDoneCondition;
	static std::mutex reportDoneMutex;

	static int numSystemCores(void)
	{
		return std::max(1u, std::thread::hardware_concurrency());
	}

	static int maxThreadIndex(void)
	{
		return numSystemCores();
	}

	// Abstract the job partition
	class ParallelForLoop
	{
	public:
		ParallelForLoop(std::function<void(int64_t)> func, int64_t maxIndex, int chunkSize)
			: mFunc(std::move(func))
			, mMaxIndex(maxIndex)
			, mChunkSize(chunkSize)
		{
			//std::cout << "ParallelForLoop, max idex=" << mMaxIndex << " Constructed.\n";
		}
        ParallelForLoop(const std::function<void(Point2i)> &f, const Point2i &count)
            : mFunc2D(f),
            mMaxIndex(count.x * count.y),
            mChunkSize(1){
            mX = count.x;
        }
	public:
		const int64_t mMaxIndex;
		const int mChunkSize;

		int64_t mNextIndex = 0;
		int mActiveWorkers = 0;
		std::function<void(int64_t)> mFunc;
        std::function<void(Point2i)> mFunc2D;
		ParallelForLoop *next = nullptr;
        int mX = -1;

	public:
		bool Finished(void) const
		{
			//std::cout << "ParallelForLoop, max idex=" << mMaxIndex << " Finished.";
			return mNextIndex >= mMaxIndex && mActiveWorkers == 0;
		}

	};

	// worker thread
	void workerThread(int tIndex)
	{
		//std::cout << "Thread " << tIndex << " starting.\n";
		threadIndex = tIndex; // thread local

		std::unique_lock<std::mutex> lock(workListMutex);
		while (!shutdowThread)
		{
			if (reportWorkerStats) //atomic
			{
				ReportThreadStats();
				if (--reporterCount == 0)
					// Once all worker threads have merged their stats, wake up main thread.
					reportDoneCondition.notify_one();
				// Now sleep again.
				workListCondition.wait(lock);
			}
			else if (!workList)
			{
				workListCondition.wait(lock);
			}
			else
			{
				// Thread wakes and does job here.
				ParallelForLoop &loop = *workList;
				int64_t indexStart = loop.mNextIndex;
				int64_t indexEnd = std::min(indexStart + loop.mChunkSize, loop.mMaxIndex);
				loop.mNextIndex = indexEnd;
				if (loop.mNextIndex == loop.mMaxIndex) workList = loop.next;
				loop.mActiveWorkers++;

				lock.unlock();
				//std::cout << "Thread " << threadIndex << "starts work on [" << indexStart << ", " << indexEnd << "]\n";
				for (uint64_t i = indexStart; i < indexEnd; ++i)
				{
					if (loop.mFunc) loop.mFunc(i);
                    else if (loop.mFunc2D) loop.mFunc2D(Point2i(i% loop.mX, i/ loop.mX));
				}
				lock.lock();
				loop.mActiveWorkers--;
				if (loop.Finished()) workListCondition.notify_all();
			}
		}
		//std::cout << "Thread " << tIndex << " exiting.\n";
	}

	// PUBLIC interface
	// Called by Main Thread
	// count: total work in a ParallelForLoop
	// chunkSize:  work for a single worker in a ParallelLoop
	void ParallelFor(std::function<void(int64_t)> func, int64_t count, int chunkSize)
	{
		// Parallel is not available or no need.
		if (threads.size() == 0 || count < chunkSize)
		{
			for (int64_t i = 0; i < count; ++i) func(i);
			return;
		}

		// Create ParallelFor object, Add it into work list.
		ParallelForLoop loop(std::move(func), count, chunkSize);
		workListMutex.lock();
		loop.next = workList;
		workList = &loop;
		workListMutex.unlock();

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

		while (!loop.Finished())
		{
			int64_t indexStart = loop.mNextIndex;
			int64_t indexEnd = std::min(indexStart + loop.mChunkSize, loop.mMaxIndex);
			loop.mNextIndex = indexEnd;
			if (loop.mNextIndex == loop.mMaxIndex) workList = loop.next;
			loop.mActiveWorkers++;

			lock.unlock(); // release lock after update loop parameter.

			//std::cout << "Main Thread " << "starts work on [" << indexStart << ", " << indexEnd << "]\n";
			for (uint64_t i = indexStart; i < indexEnd; ++i)
			{
				if (loop.mFunc) loop.mFunc(i);
			}
			lock.lock();
			loop.mActiveWorkers--;
		}
	}

    void ParallelFor2D(std::function<void(Point2i)> func, const Point2i &count)
    {
		// Parallel is not available or no need.
        if (threads.empty() || count.x * count.y <= 1)
        {
            for (int y = 0; y < count.y; ++y)
                for (int x = 0; x < count.x; ++x) func(Point2i(x, y));
            return;
        }

		// Create ParallelFor object, Add it into work list.
		ParallelForLoop loop(std::move(func), count);
		workListMutex.lock();
		loop.next = workList;
		workList = &loop;
		workListMutex.unlock();

		std::unique_lock<std::mutex> lock(workListMutex);
		workListCondition.notify_all();

		while (!loop.Finished())
		{
			int64_t indexStart = loop.mNextIndex;
			int64_t indexEnd = std::min(indexStart + loop.mChunkSize, loop.mMaxIndex);
			loop.mNextIndex = indexEnd;
			if (loop.mNextIndex == loop.mMaxIndex) workList = loop.next;
			loop.mActiveWorkers++;

			lock.unlock(); // release lock after update loop parameter.

			//std::cout << "Main Thread " << "starts work on [" << indexStart << ", " << indexEnd << "]\n";
			for (uint64_t i = indexStart; i < indexEnd; ++i)
			{
                if (loop.mFunc2D) loop.mFunc2D(Point2i(i % loop.mX, i / loop.mX));
			}
			lock.lock();
			loop.mActiveWorkers--;
		}
	}

//< Debug in serialize mode
//#define DEBUG_SERIALIZE


	//< Called in Main Thread
	void ParallelInit(void)
	{
		if (threads.size() > 0)
		{
			LOG(ERROR) << "Thread Array is NOT empty. Parallel module init Failed";
			return;
		}
		int nThreads = maxThreadIndex();
		threadIndex = 0; // thread local variable.

#if defined(DEBUG_SERIALIZE) 
		nThreads = 1; //debug only.
#endif

		for (int i = 0; i < nThreads - 1; ++i)
			threads.push_back(std::thread(workerThread, i + 1));
		LOG(WARNING) << "Parallel module init Ok. Spaw " << nThreads-1 << " worker threads.";
	}

	//< Called in Main Thread
	//< Main Thread would wait for all worker threads exit.
	void ParallelCleanup(void)
	{
		if (threads.size() == 0)
		{
			LOG(INFO) << "NO worker threads spawed. Parallel module clean up Ok.";
			return;
		}

		{
			std::lock_guard<std::mutex> lock(workListMutex);
			shutdowThread = true;
			workListCondition.notify_all();
		}

		for (std::thread &thread : threads) thread.join();
		threads.erase(threads.begin(), threads.end());
		shutdowThread = false;
		LOG(INFO) << "Parallel module clean up Ok.";
	}

	// Called in Main Thread
	void MergeWorkerThreadStats()
	{
		std::unique_lock<std::mutex> lock(workListMutex);
		std::unique_lock<std::mutex> doneLock(reportDoneMutex);
		// Set up state so that the worker threads will know that we would like
		// them to report their thread-specific stats when they wake up.
		reportWorkerStats = true;
		reporterCount = threads.size();

		// Wake up the worker threads.
		workListCondition.notify_all();

		// Wait for all of them to merge their stats.
		reportDoneCondition.wait(lock, []() { return reporterCount == 0; });

		reportWorkerStats = false;
	}



//#define LOCAL_TEST

#ifdef LOCAL_TEST
#include <assert.h>
#include "progressreporter.h"
int main()
{
#define N (1024 * 1024 * 1)
    static int64_t a[N] = {0};
    parallelInit();

    // Main Thread also does jobs
    {
        ProgressReporter reporter(N, "Progressing");
        ParallelFor([&](int64_t i) {
             a[i] = i;
             reporter.update();
        }, N, 4096);
        reporter.done();
    }

    MergeWorkerThreadStats();
    ReportThreadStats();
    PrintStats(stdout);
    ClearStats();
    parallelCleanup();

    for (int64_t i = 0; i < N; ++i)
        assert(a[i] == i);

    system("pause");
    return 0;
}

#endif
}