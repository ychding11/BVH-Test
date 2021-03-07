#pragma once

#include <bvh/vector.hpp> 

#include "Log.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

// UI Control
struct Setting
{
    int  width;
    int  height;
    bool fitToWindow;
    bool statistic;

    Scalar *data;

    struct
    {
        Vector3 eye;
        Vector3 dir;
        Vector3 up;
        Scalar  fov;
    } camera ;

    Setting& operator =(const Setting &setting)
    {
        assert((this->data == setting.data) && this->data == nullptr);
        this->width = setting.width;
        this->height = setting.height;
        this->fitToWindow = setting.fitToWindow;
        this->statistic = setting.statistic;
        this->camera.eye = setting.camera.eye;
        this->camera.dir = setting.camera.dir;
        this->camera.up = setting.camera.up;
        this->camera.fov= setting.camera.fov;
        return *this;
    }

    bool operator ==(const Setting &setting)
    {
        if (width != setting.width || height != setting.height)
            return false;
        if (fitToWindow != setting.fitToWindow || statistic != setting.statistic)
            return false;
        return true;
    }

    Setting(bool a = true)
        : width(1280)
        , height(720)
        , fitToWindow(true)
        , statistic(false)
        , data(nullptr)
    {
        camera.eye = Vector3(0, 0.9, 2.5);
        camera.dir = Vector3(0, 0.001, -1);
        camera.up  = Vector3(0, 1, 0);
        camera.fov = 60;
        fitToWindow = a ;
    }
};

extern Setting gSettings;

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

        void* QueryTaskData(TaskHandle handle)
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
                return task->userData;
            return nullptr;
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
                    Log("complete a task: handle={}",task->handle);
                }
            }
        }


    };
//}

// implement in external source code
void Rendering(void *taskUserData);

inline TaskHandle StartRenderingTask(Setting &setting)
{
    static Setting local(false);
    if (local == setting)
    {
        return Invalid_Task_Handle;
    }
    else
    {
        local = setting;
    }
    Setting *temp = new Setting();
    *temp = setting;

    Task *task = new Task;
    task->func = Rendering;
    task->userData = temp;
    task->status = TaskStatus::Created;

    Log("schedule a task: handle={}",task->handle);
    TaskScheduler::GetScheduler()->Schedule(task);
    return task->handle;
}

inline bool TaskDone(TaskHandle handle)
{
    if (handle == Invalid_Task_Handle) return false;
    return TaskScheduler::GetScheduler()->QueryTaskStatus(handle) == TaskStatus::Completed;
}


inline float* GetRenderingResult(TaskHandle handle)
{
    void *data = TaskScheduler::GetScheduler()->QueryTaskData(handle);
    if (data == nullptr)
    {
        Err("Task output corrupted.");
        return nullptr;
    }
    Setting &temp = *(reinterpret_cast<Setting*>(data));
    
    return temp.data;;
}
