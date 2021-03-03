#include "interface.h"

Setting settings;
float  *result = nullptr;

//{

    TaskHandle Task::s_count = 0;
    thread_local uint32_t TaskScheduler::m_threadIndex;

    static TaskScheduler scheduler;
    TaskScheduler* TaskScheduler::GetScheduler()
    {
        return &scheduler;
    }

//}
//TaskHandle StartRenderingTask(Setting &setting)
//{
//    Task *task = new Task;
//    task->func = Rendering;
//    task->userData = &settings;
//    task->status = TaskStatus::Created;
//
//    TaskScheduler::GetScheduler()->Schedule(task);
//    return task->handle;
//}


