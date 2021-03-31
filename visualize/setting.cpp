#include "setting.h"
#include <map> 

std::string RenderSetting::str() const
{
    std::stringstream ss;
    ss
        << "width : " << width << "\n"
        << "height: " << height << "\n"
        << "statistic: " << statistic << "\n"
        << "3D model: " << modelPath << "\n"
        << camera
        ;

    return ss.str();
}

RenderSetting& RenderSetting::operator=(const RenderSetting &setting)
{
    assert((this->data == setting.data) && this->data == nullptr);
    this->onlyUseForIndentification = setting.onlyUseForIndentification;
    this->width = setting.width;
    this->height = setting.height;
    this->statistic = setting.statistic;
    this->bvhBuilderType = setting.bvhBuilderType;
    this->modelPath = setting.modelPath;
    //this->camera.eye = setting.camera.eye;
    //this->camera.dir = setting.camera.dir;
    //this->camera.up = setting.camera.up;
    //this->camera.fov = setting.camera.fov;
    this->camera = setting.camera;
    return *this;
}

bool  RenderSetting::operator==(const RenderSetting &setting)
{
    if (onlyUseForIndentification != setting.onlyUseForIndentification)
        return false;
    if (width != setting.width || height != setting.height)
        return false;
    if (statistic != setting.statistic)
        return false;
    if (bvhBuilderType != setting.bvhBuilderType)
        return false;
    if (modelPath != setting.modelPath)
        return false;
    if (camera != setting.camera)
        return false;
    return true;
}

 RenderSetting::RenderSetting(bool a)
    : width(1280)
    , height(720)
    , statistic(false)
    , bvhBuilderType(0)
    , modelPath(".\\scene\\cornell_box.obj")
    , data(nullptr)
    , onlyUseForIndentification(a == true ? 1 : 0)
{
    camera.eye = Vector3(0, 0.9, 3.5);
    camera.dir = Vector3(0, 0.001, -1);
    camera.up = Vector3(0, 1, 0);
    camera.fov = 60;
}


static std::map<BVHBuilderType, std::string> g_BVHBuilderNames = 
{
    {Binned_SAH, "binned_sah"},
    {Sweep_SAH, "sweep_sah"},
    {Spatial_Split, "spatial_split"},
    {Locally_Ordered_Clustering, "locally_ordered_clustering"},
    {Linear, "linear"},
};

std::string BvhBuilderTypeStr(BVHBuilderType type)
{
    return g_BVHBuilderNames[type];
}
