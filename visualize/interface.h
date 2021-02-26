#pragma once

#include "Log.h"


// UI Control
struct Setting
{
    int testIndex;
    int objectPerAxis;
    int width;
    int height;
    float positionOffset;

    Setting()
        : testIndex(0)
        , objectPerAxis(10)
        , width(1280)
        , height(720)
        , positionOffset(0.f)
    {}
};

extern Setting settings;


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
