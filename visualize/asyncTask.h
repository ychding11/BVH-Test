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

        Task()
        {
            func = nullptr;
            userData = nullptr;
            status = TaskStatus::Invalid;
            handle = s_count++;
        }

    private:
        static TaskHandle s_count;
    };

    class TaskScheduler
    {
    public:

        static TaskScheduler* GetScheduler();

        TaskScheduler() : m_shutdown(false)
        {
            m_threadIndex = 0;

            m_group = new TaskGroup();
            m_group->free = true;
            m_group->ref = 0;

            m_workers.resize(std::thread::hardware_concurrency() <= 1 ? 1 : std::thread::hardware_concurrency() - 1);
            for (uint32_t i = 0; i < m_workers.size(); i++)
            {
                m_workers[i] = new Worker();
                m_workers[i]->wakeup = false;
                m_workers[i]->thread = new std::thread(workerThread, this, m_workers[i], i + 1);
            }
        }

        ~TaskScheduler()
        {
            m_shutdown = true;
            for (uint32_t i = 0; i < m_workers.size(); i++)
            {
                Worker &worker = *(m_workers[i]);
                assert(worker.thread);
                worker.wakeup = true;
                worker.cv.notify_one();
                if (worker.thread->joinable()) worker.thread->join();
                worker.thread->~thread();
                free(worker.thread);
                worker.~Worker();
            }

            m_group->~TaskGroup();
            free(m_group);
        }

        void* QueryTaskData(TaskHandle handle);

        TaskStatus QueryTaskStatus(TaskHandle handle);

        void Schedule(Task *task);

    private:

        struct Spinlock
        {
            void lock() { while (m_lock.test_and_set(std::memory_order_acquire)) {} }
            void unlock() { m_lock.clear(std::memory_order_release); }

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
        return nullptr;
    }
    
    Log("task {} data is got.", handle);
    return data;;
}
