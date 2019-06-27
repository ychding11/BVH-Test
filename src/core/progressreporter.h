#ifndef PROGRESS_REPORT_H_
#define PROGRESS_REPORT_H_

#include "mei.h"

#include <atomic>
#include <thread>
#include <chrono>
#include <string>

namespace mei
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


}
#endif
