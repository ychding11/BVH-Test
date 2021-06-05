#pragma once

#include <sstream> 
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "Log.h"

//namespace
//{
    struct MemTag
    {
        enum
        {
            Default,
            BitImage,
            BVH,
            Matrix,
            Mesh,
            Count
        };
    };

    enum TaskStatus
    {
        Invalid = -1,
        Created = 0,
        Scheduled,
        Running,
        Completed,
        Count
    };

    typedef uint32_t TaskHandle;

    #define Invalid_Task_Handle (~0U)

    struct Task
    {
        TaskHandle handle;
        std::atomic<TaskStatus> status;

        void(*func)(void *taskUserData);
        void *userData; // Passed to func as taskUserData.
        std::string profilerData;

        //< refine required
        Task()
            : func(nullptr)
            , userData(nullptr)
            , status(TaskStatus::Invalid)
            , handle(s_count++)
        {
            //func = nullptr;
            //userData = nullptr;
            //status = TaskStatus::Invalid;
            //handle = s_count++;
        }

    private:
        static TaskHandle s_count;
    };

    class TaskScheduler
    {
    public:

        static TaskScheduler* GetScheduler();

    public:
        TaskScheduler();        

        ~TaskScheduler();

        void Schedule(Task *task);

        void* QueryTaskData(TaskHandle handle);

        Task* QueryTask(TaskHandle handle);

        TaskStatus QueryTaskStatus(TaskHandle handle);

    private:

        struct Spinlock
        {
            void lock()
            {
                while (m_lock.test_and_set(std::memory_order_acquire)) {}
            }
            void unlock()
            {
                m_lock.clear(std::memory_order_release);
            }

        private:
            std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
        };

        struct Worker
        {
            std::thread *thread = nullptr;
            std::mutex mutex;
            std::condition_variable cv;
            std::atomic<bool> wakeup;
        };

        struct TaskGroup
        {
            std::atomic<bool> free;
            std::vector<Task*> queue; // Items are never removed. queueHead is incremented to pop items.
            uint32_t queueHead = 0;
            Spinlock queueLock;
            std::atomic<uint32_t> ref; // Increment when a task is enqueued, decrement when a task finishes.
            void *userData;
        };

        TaskGroup *m_group;
        std::atomic<bool> m_shutdown;
        std::vector<Worker*> m_workers;

        static thread_local uint32_t m_threadIndex;
        static void workerThread(TaskScheduler *scheduler, Worker *worker, uint32_t threadIndex);
    };
//}

inline bool TaskDone(TaskHandle handle)
{
    if (handle == Invalid_Task_Handle) return false;
    return TaskScheduler::GetScheduler()->QueryTaskStatus(handle) == TaskStatus::Completed;
}

inline void* FetchRenderTaskData(TaskHandle handle)
{
    void *data = TaskScheduler::GetScheduler()->QueryTaskData(handle);
    if (data == nullptr)
    {
        Err("task {} output is corrupted.", handle);
    }
    return data;;
}
