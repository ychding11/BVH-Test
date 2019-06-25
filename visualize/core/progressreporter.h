#ifndef PROGRESS_REPORT_H_
#define PROGRESS_REPORT_H_

#include <atomic>
#include <thread>
#include <chrono>
#include <string>

namespace smallpt
{

class ProgressReporter
{ 
private:
    const  int64_t totalWork;
    const std::string title;
    const std::chrono::system_clock::time_point startTime;
    std::atomic<int64_t> workDone;
    std::atomic<bool> exitThread;
    std::thread updateThread;

public:

    ProgressReporter(int64_t totalWork, const std::string &title);

   ~ProgressReporter();

    void update(int64_t num = 1)
    {
        if (0 == num) return;
        workDone += num;
    }
    void done(void);

    float elapsedMs() const
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        return (float)elapsedMs;
    }

private:
    void printBar(void);
};


#ifdef __GNUG__
#define RUN_ON_LINUX
#else
#endif  // __GNUG__

typedef float Float;
#define Infinity std::numeric_limits<Float>::infinity()

// Check OS & Platform
#if defined(_WIN32) || defined(_WIN64)
#define OS_IS_WINDOWS
#if defined(__MINGW32__)  // Defined for both 32 bit/64 bit MinGW
#define ENV_IS_MINGW
#elif defined(_MSC_VER)
#define ENV_IS_MSVC
#endif
#elif defined(__linux__)
#define OS_IS_LINUX
#elif defined(__APPLE__)
#define OS_IS_OSX
#elif defined(__OpenBSD__)
#define OS_IS_OPENBSD
#elif defined(__FreeBSD__)
#define OS_IS_FREEBSD
#endif

}
#endif
