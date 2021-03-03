#pragma once

#include <bvh/vector.hpp> 

#include "Log.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

// UI Control
struct Setting
{
    int testIndex;
    int width;
    int height;

    struct
    {
        Vector3 eye;
        Vector3 dir;
        Vector3 up;
        Scalar  fov;
    } camera ;

    bool fitToWindow;
    Setting()
        : testIndex(0)
        , width(1280)
        , height(720)
        , fitToWindow(true)
    {
        camera.eye = Vector3(0, 0.9, 2.5);
        camera.dir = Vector3(0, 0.001, -1);
        camera.up  = Vector3(0, 1, 0);
        camera.fov = 60;
    }
};

extern Setting settings;
extern float *result;


int EntryPointMain(int argc, char** argv);

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

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

        TaskStatus QueryTaskStatus(TaskHandle handle)
        {
            Task *task = nullptr;
            m_group->queueLock.lock();
            for (uint32_t i = 0; i < m_group->queueHead; ++i)
            {
                task = m_group->queue[i];
                if (task->handle == handle)
                    break;
            }
            m_group->queueLock.unlock();
            if (task)
                return task->status.load();
            return TaskStatus::Invalid;
        }

        void Schedule(Task *task)
        {
            TaskGroup &group = *m_group;
            group.queueLock.lock();
            task->status = TaskStatus::Scheduled;
            group.queue.push_back(task);
            group.queueLock.unlock();
            group.ref++;

            // Wake up a worker to run this task.
            for (uint32_t i = 0; i < m_workers.size(); i++)
            {
                m_workers[i]->wakeup = true;
                m_workers[i]->cv.notify_one();
            }
        }

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
        static void workerThread(TaskScheduler *scheduler, Worker *worker, uint32_t threadIndex)
        {
            m_threadIndex = threadIndex;
            std::unique_lock<std::mutex> lock(worker->mutex);
            for (;;)
            {
                worker->cv.wait(lock, [=] { return worker->wakeup.load(); });
                worker->wakeup = false;
                //for (;;)
                {
                    if (scheduler->m_shutdown) return;

                    TaskGroup *group = nullptr;
                    Task *task = nullptr;
                    {
                        group = scheduler->m_group;
                        if (group->ref == 0) continue;

                        group->queueLock.lock();
                        if (group->queueHead < group->queue.size())
                        {
                            task = group->queue[group->queueHead++];
                            group->queueLock.unlock();
                        }
                        else
                            group->queueLock.unlock();
                    }
                    if (!task) break;
                    task->status = TaskStatus::Running;
                    task->func(task->userData);
                    task->status = TaskStatus::Completed;
                    group->ref--;
                }
            }
        }


    };
//}

// implement in external source code
void Rendering(void *taskUserData);

inline TaskHandle StartRenderingTask(Setting &setting)
{
    Task *task = new Task;
    task->func = Rendering;
    task->userData = &settings;
    task->status = TaskStatus::Created;

    TaskScheduler::GetScheduler()->Schedule(task);
    return task->handle;
}

inline bool TaskDone(TaskHandle handle)
{
    return TaskScheduler::GetScheduler()->QueryTaskStatus(handle) == TaskStatus::Completed;
}

inline bool RenderingTaskDone()
{
    return false;
}


inline float* GetRenderingResult()
{
    return result;
}

#define XA_MULTITHREADED 0

#if XA_MULTITHREADED
class TaskScheduler
{
public:
    TaskScheduler() : m_shutdown(false)
    {
        m_threadIndex = 0;
        // Max with current task scheduler usage is 1 per thread + 1 deep nesting, but allow for some slop.
        m_maxGroups = std::thread::hardware_concurrency() * 4;
        m_groups = XA_ALLOC_ARRAY(MemTag::Default, TaskGroup, m_maxGroups);
        for (uint32_t i = 0; i < m_maxGroups; i++) {
            new (&m_groups[i]) TaskGroup();
            m_groups[i].free = true;
            m_groups[i].ref = 0;
            m_groups[i].userData = nullptr;
        }
        m_workers.resize(std::thread::hardware_concurrency() <= 1 ? 1 : std::thread::hardware_concurrency() - 1);
        for (uint32_t i = 0; i < m_workers.size(); i++) {
            new (&m_workers[i]) Worker();
            m_workers[i].wakeup = false;
            m_workers[i].thread = XA_NEW_ARGS(MemTag::Default, std::thread, workerThread, this, &m_workers[i], i + 1);
        }
    }

    ~TaskScheduler()
    {
        m_shutdown = true;
        for (uint32_t i = 0; i < m_workers.size(); i++) {
            Worker &worker = m_workers[i];
            XA_DEBUG_ASSERT(worker.thread);
            worker.wakeup = true;
            worker.cv.notify_one();
            if (worker.thread->joinable())
                worker.thread->join();
            worker.thread->~thread();
            XA_FREE(worker.thread);
            worker.~Worker();
        }
        for (uint32_t i = 0; i < m_maxGroups; i++)
            m_groups[i].~TaskGroup();
        XA_FREE(m_groups);
    }

    uint32_t threadCount() const
    {
        return max(1u, std::thread::hardware_concurrency()); // Including the main thread.
    }

    // userData is passed to Task::func as groupUserData.
    TaskGroupHandle createTaskGroup(void *userData = nullptr, uint32_t reserveSize = 0)
    {
        // Claim the first free group.
        for (uint32_t i = 0; i < m_maxGroups; i++) {
            TaskGroup &group = m_groups[i];
            bool expected = true;
            if (!group.free.compare_exchange_strong(expected, false))
                continue;
            group.queueLock.lock();
            group.queueHead = 0;
            group.queue.clear();
            group.queue.reserve(reserveSize);
            group.queueLock.unlock();
            group.userData = userData;
            group.ref = 0;
            TaskGroupHandle handle;
            handle.value = i;
            return handle;
        }
        XA_DEBUG_ASSERT(false);
        TaskGroupHandle handle;
        handle.value = UINT32_MAX;
        return handle;
    }

    void run(TaskGroupHandle handle, const Task &task)
    {
        XA_DEBUG_ASSERT(handle.value != UINT32_MAX);
        TaskGroup &group = m_groups[handle.value];
        group.queueLock.lock();
        group.queue.push_back(task);
        group.queueLock.unlock();
        group.ref++;
        // Wake up a worker to run this task.
        for (uint32_t i = 0; i < m_workers.size(); i++) {
            m_workers[i].wakeup = true;
            m_workers[i].cv.notify_one();
        }
    }

    void wait(TaskGroupHandle *handle)
    {
        if (handle->value == UINT32_MAX) {
            XA_DEBUG_ASSERT(false);
            return;
        }
        // Run tasks from the group queue until empty.
        TaskGroup &group = m_groups[handle->value];
        for (;;) {
            Task *task = nullptr;
            group.queueLock.lock();
            if (group.queueHead < group.queue.size())
                task = &group.queue[group.queueHead++];
            group.queueLock.unlock();
            if (!task)
                break;
            task->func(group.userData, task->userData);
            group.ref--;
        }
        // Even though the task queue is empty, workers can still be running tasks.
        while (group.ref > 0)
            std::this_thread::yield();
        group.free = true;
        handle->value = UINT32_MAX;
    }

    static uint32_t currentThreadIndex() { return m_threadIndex; }

private:
    struct TaskGroup
    {
        std::atomic<bool> free;
        Array<Task> queue; // Items are never removed. queueHead is incremented to pop items.
        uint32_t queueHead = 0;
        Spinlock queueLock;
        std::atomic<uint32_t> ref; // Increment when a task is enqueued, decrement when a task finishes.
        void *userData;
    };

    struct Worker
    {
        std::thread *thread = nullptr;
        std::mutex mutex;
        std::condition_variable cv;
        std::atomic<bool> wakeup;
    };

    TaskGroup *m_groups;
    Array<Worker> m_workers;
    std::atomic<bool> m_shutdown;
    uint32_t m_maxGroups;
    static thread_local uint32_t m_threadIndex;

    static void workerThread(TaskScheduler *scheduler, Worker *worker, uint32_t threadIndex)
    {
        m_threadIndex = threadIndex;
        std::unique_lock<std::mutex> lock(worker->mutex);
        for (;;) {
            worker->cv.wait(lock, [=] { return worker->wakeup.load(); });
            worker->wakeup = false;
            for (;;) {
                if (scheduler->m_shutdown)
                    return;
                // Look for a task in any of the groups and run it.
                TaskGroup *group = nullptr;
                Task *task = nullptr;
                for (uint32_t i = 0; i < scheduler->m_maxGroups; i++) {
                    group = &scheduler->m_groups[i];
                    if (group->free || group->ref == 0)
                        continue;
                    group->queueLock.lock();
                    if (group->queueHead < group->queue.size()) {
                        task = &group->queue[group->queueHead++];
                        group->queueLock.unlock();
                        break;
                    }
                    group->queueLock.unlock();
                }
                if (!task)
                    break;
                task->func(group->userData, task->userData);
                group->ref--;
            }
        }
    }
};

thread_local uint32_t TaskScheduler::m_threadIndex;
#else
#endif
