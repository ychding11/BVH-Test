#include "asyncTask.h"

//{

    TaskHandle Task::s_count = 0;
    thread_local uint32_t TaskScheduler::m_threadIndex;

    static TaskScheduler scheduler;
    TaskScheduler* TaskScheduler::GetScheduler()
    {
        return &scheduler;
    }


    TaskScheduler::TaskScheduler()
        : m_shutdown(false)
    {
        m_threadIndex = 0;

        m_group = new TaskGroup();
        m_group->free = true;
        m_group->ref = 0;

        //m_workers.resize(std::thread::hardware_concurrency() <= 1 ? 1 : std::thread::hardware_concurrency() - 1);
        m_workers.resize(1);
        for (uint32_t i = 0; i < m_workers.size(); i++)
        {
            m_workers[i] = new Worker();
            m_workers[i]->wakeup = false;
            m_workers[i]->thread = new std::thread(workerThread, this, m_workers[i], i + 1);
        }
    }
    TaskScheduler::~TaskScheduler()
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

    void TaskScheduler::workerThread(TaskScheduler *scheduler, Worker *worker, uint32_t threadIndex)
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

    void* TaskScheduler::QueryTaskData(TaskHandle handle)
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

    TaskStatus TaskScheduler::QueryTaskStatus(TaskHandle handle)
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

    void TaskScheduler::Schedule(Task *task)
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

//}


