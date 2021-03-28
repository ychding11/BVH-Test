#include "interface.h"

RenderSetting gSettings;

//{

    TaskHandle Task::s_count = 0;
    thread_local uint32_t TaskScheduler::m_threadIndex;

    static TaskScheduler scheduler;
    TaskScheduler* TaskScheduler::GetScheduler()
    {
        return &scheduler;
    }

//}


