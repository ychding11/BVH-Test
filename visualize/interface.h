#pragma once

#include <sstream> 
#include <bvh/vector.hpp> 

#include "Log.h"
#include "camera.h"
#include "asyncTask.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

// UI Control
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

    std::string str() const
    {
        std::stringstream ss;
        ss
            << "width : " << width << "\n"
            << "height: " << height << "\n"
            << "statistic: " << statistic << "\n"
            << camera
            ;

        return ss.str();
    }

    RenderSetting& operator =(const RenderSetting &setting)
    {
        assert((this->data == setting.data) && this->data == nullptr);
        this->onlyUseForIndentification = setting.onlyUseForIndentification;
        this->width = setting.width;
        this->height = setting.height;
        this->statistic = setting.statistic;
        this->bvhBuilderType = setting.bvhBuilderType;
        this->camera.eye = setting.camera.eye;
        this->camera.dir = setting.camera.dir;
        this->camera.up = setting.camera.up;
        this->camera.fov= setting.camera.fov;
        return *this;
    }

    bool operator ==(const RenderSetting &setting)
    {
        if (onlyUseForIndentification != setting.onlyUseForIndentification)
            return false;
        if (width != setting.width || height != setting.height)
            return false;
        if (statistic != setting.statistic)
            return false;
        if (bvhBuilderType != setting.bvhBuilderType)
            return false;
        if (camera != setting.camera)
            return false;
        return true;
    }

    RenderSetting(bool a = true)
        : width(1280)
        , height(720)
        , statistic(false)
        , bvhBuilderType(0)
        , data(nullptr)
        , onlyUseForIndentification(a == true ? 1 : 0)
    {
        camera.eye = Vector3(0, 0.9, 3.5);
        camera.dir = Vector3(0, 0.001, -1);
        camera.up  = Vector3(0, 1, 0);
        camera.fov = 60;
    }
};

extern RenderSetting gSettings;

int EntryPointMain(int argc, char** argv);

//#include <thread>
//#include <mutex>
//#include <atomic>
//#include <condition_variable>


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


inline float* GetRenderingResult(TaskHandle handle)
{
    void *data = TaskScheduler::GetScheduler()->QueryTaskData(handle);
    if (data == nullptr)
    {
        Err("task {} output is corrupted.", handle);
        return nullptr;
    }
    RenderSetting &temp = *(reinterpret_cast<RenderSetting*>(data));
    
    Log("task {} output is got.", handle);
    return temp.data;;
}

