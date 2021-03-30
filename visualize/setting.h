#pragma once

#include <sstream> 
#include <bvh/vector.hpp> 

#include "Log.h"
#include "camera.h"
#include "asyncTask.h"

using Scalar  = float;
using Vector3 = bvh::Vector3<Scalar>;

enum BVHBuilderType
{
    Invalid_Type =  -1,
    Binned_SAH = 0,                 //binned_sah
    Sweep_SAH = 1,                  //sweep_sah
    Spatial_Split = 2,              //spatial_split
    Locally_Ordered_Clustering = 3, //locally_ordered_clustering
    Linear = 4,                     //linear
    Builder_Count
};

std::string BvhBuilderTypeStr(BVHBuilderType type);

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

