#pragma once

#include <sstream> 
#include <bvh/vector.hpp> 

#include "Log.h"
#include "camera.h"
#include "asyncTask.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

struct RenderSetting
{
    int  onlyUseForIndentification;
    int  width;
    int  height;
    int  bvhBuilderType;
    bool statistic;
    Camera camera;

    // the output of current setting
    Scalar *data;

    std::string str() const;

    RenderSetting& operator =(const RenderSetting &setting);

    bool operator ==(const RenderSetting &setting);

    RenderSetting(bool a = true);
};

int EntryPointMain(int argc, char** argv);

// implement in external source code
void Rendering(void *taskUserData);

inline TaskHandle StartRenderingTask(RenderSetting &setting)
{
    static RenderSetting local(false);
    if (local == setting) // identical to the previous setting, no need to start a new task
    {
        return Invalid_Task_Handle;
    }
    else
    {
        local = setting;
    }

    RenderSetting *temp = new RenderSetting();
    *temp = setting;

    Task *task = new Task;
    task->func = Rendering;
    task->userData = temp;
    task->status = TaskStatus::Created;

    TaskScheduler::GetScheduler()->Schedule(task);
    Log("schedule a task: handle={}",task->handle);
    return task->handle;
}
