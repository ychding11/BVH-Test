#include <cstdio>
#include <vector>
#include <queue> 
#include <map>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include <math.h>

// third-party libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>    
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gui.h"

#include "setting.h"
#include "Renderer.h"
#include "IconsFontAwesome4.h"

#include "profiler.h"

//#pragma warning(push)
#pragma warning(disable : 4244) // conversion from 'int' to 'float', possible loss of data
//#pragma warning(pop)


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
    return task->handle;
}

//< CompletedTaskMap record map { task handle <--> task pointer }
//< previous design record map { task handle <--> task data pointer }, it is a bad idea in practice.
//< In my design, task is scheduled serially. It always requires to query info of a task which is not
//< beloged to task data. The operation requires lock. So record task pointer directly can void this 
//< lock operation.
typedef std::map<TaskHandle, Task*> CompletedTaskMap;
static CompletedTaskMap g_CompletedTasks;

// options has no relation with rendering
struct DisplayOption
{
    bool flipVertical;
    bool showSplitWindow;
    bool fitToWindow;
    bool showProfilerData;
    TaskHandle completeTaskHandle;
    DisplayOption()
        : flipVertical(false)
        , showSplitWindow(false)
        , fitToWindow(true)
        , showProfilerData(true)
        , completeTaskHandle(Invalid_Task_Handle)
    { }
};


#include <nfd.h>
static std::string ModelPathDialog()
{
    nfdchar_t *filename = nullptr;
    nfdresult_t result = NFD_OpenDialog("obj", nullptr, &filename);

    if (result != NFD_OKAY)
    {
        if (filename) free(filename);
        return "";
    }
    return std::string(filename);
}

static void ShowHelpTip(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void update(float secondsElapsed, GLFWwindow *window)
{
    static bool keyPressed = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GL_TRUE);

    //Camera Movement
    if (glfwGetKey(window, 'W'))
    {
        keyPressed = true;
    }
    else if (glfwGetKey(window, 'S'))
    {
        keyPressed = true;
    }
    if (glfwGetKey(window, 'A'))
    {
        keyPressed = true;
    }
    else if (glfwGetKey(window, 'D'))
    {
        keyPressed = true;
    }

    //Mouse Handling
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && ImGui::IsMouseDown(0))
    {
        ImVec2 mouseDelta = ImGui::GetMouseDragDelta();
        ImGui::ResetMouseDragDelta();
    }
}

static void drawMenuBar(RenderSetting &setting, DisplayOption & displayOption)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(ICON_FA_CUBE " Model"))
        {
            if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open..."))
            {
                auto path = ModelPathDialog();
                if (!path.empty()) setting.modelPath = path;
                Log("model : {}", path)
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_CAMERA " Camera"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_EYE " View"))
        {
            ImGui::Checkbox("flip vertical", &displayOption.flipVertical);
            ImGui::Checkbox("fit window",    &displayOption.fitToWindow);
            ImGui::Checkbox("split window",  &displayOption.showSplitWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_WINDOWS " Settings"))
        {
            ImGui::Checkbox("profiler data",  &displayOption.showProfilerData);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleVar();
}

static bool initDockLayout = true;

const char *testOptionsWindowName = ICON_FA_GLOBE "Test Options";
const char *profileWindowName     = ICON_FA_GLOBE "Profile Data";
const char *statusWindowName      = ICON_FA_GLOBE "Status";
const char *mainWindowName        = ICON_FA_GLOBE "Main";
void drawDockWindow()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("DockSpaceWindow", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
    ImGui::PopStyleVar(3);
    ImGuiID dockSpaceId = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    if (initDockLayout)
    {
        ImGuiID dockLeftId  = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.20f, nullptr, &dockSpaceId);
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.30f, nullptr, &dockSpaceId);
        ImGuiID dockRightBottomId = ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.50f, nullptr, &dockRightId);
        ImGui::DockBuilderDockWindow(testOptionsWindowName, dockLeftId);
        ImGui::DockBuilderDockWindow(statusWindowName, dockRightId);
        ImGui::DockBuilderDockWindow(profileWindowName, dockRightBottomId);
        ImGui::DockBuilderDockWindow(mainWindowName, dockSpaceId);
        ImGui::DockBuilderFinish(dockSpaceId);
        initDockLayout = false;
    }
    ImGui::End();

}

#define BACKGROUND_IMAGE "../image/waiting.png" 
void GUIModeMain(RenderSetting &setting)
{
    Log("Enter GUI Mode");


    int width  = setting.width;
    int height = setting.height;

    GLFWwindow *window;
    glfwInit();
    window = glfwCreateWindow(width, height, "BVH-Lab", 0, 0);
    glfwSetWindowPos(window, 300, 100);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetCursorPos(window, 0, 0);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // turn off vsync
    glewInit();


#if __APPLE__
// GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    */
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    GUI::Setup(window, glsl_version);

    Quad::Renderer quadRender;

    int width_, height_;
    intptr_t waitingTexture = quadRender.LoadTexture(BACKGROUND_IMAGE, width_, height_);

    //< Since it always sequentially schedule task. Pending Queue can be removed.
    //< It can be designed to previous task finished, following task begins.
    TaskHandle activeTaskHandle = Invalid_Task_Handle;

    DisplayOption displayOption;
    double lastTime = glfwGetTime();
    double curTime  = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        GUI::BeginFrame();

        drawMenuBar(setting, displayOption);
        drawDockWindow();

        {
            //static int bvhBuilderName;
            ImGui::Begin(testOptionsWindowName, &displayOption.showSplitWindow);
                ImGui::Checkbox("statistic",  &setting.statistic);
                ImGui::SliderFloat("vertical fov", &setting.camera.fov, 30.0f, 80.f);

                ImGui::Separator();
                if (ImGui::CollapsingHeader("bvh_builder_type"))
                {
                    ImGui::RadioButton("binned_sah", &setting.bvhBuilderType, Binned_SAH);
                    ImGui::RadioButton("sweep_sah", &setting.bvhBuilderType, Sweep_SAH);
                    ImGui::RadioButton("spatial_split", &setting.bvhBuilderType, Spatial_Split);
                    ImGui::RadioButton("locally_ordered_clustering", &setting.bvhBuilderType, Locally_Ordered_Clustering);
                    ImGui::RadioButton("linear", &setting.bvhBuilderType, Linear);
                }
                ImGui::Separator();
                bool newTaskCheckable = !(activeTaskHandle == Invalid_Task_Handle); 
                if (ImGui::Checkbox("new task", &newTaskCheckable)) //< checkbox should be disabled when previous task NOT completed.
                {
                    TaskHandle handle = Invalid_Task_Handle;
                    if (activeTaskHandle == Invalid_Task_Handle)
                        handle = StartRenderingTask(setting);
                    if (handle != Invalid_Task_Handle)
                    {
                        activeTaskHandle = handle;
                        Log("[Main Thread]: enque waiting task : handle={}", handle);
                    }
                }
            ImGui::End();


            ImGui::Begin(statusWindowName, &displayOption.showSplitWindow);
                ImGui::BulletText("fps %.3f ms/frame (%.1f FPS)", 33.33f, 1000.0f / 33.33f);
                ImGui::BulletText("completed rendering task count: %d", g_CompletedTasks.size());
                ImGui::Separator();
                if (ImGui::CollapsingHeader("complete_task_list"))
                {
                    for (auto it = g_CompletedTasks.begin(); it != g_CompletedTasks.end(); ++it)
                    {
                        std::stringstream ss;
                        ss << "Task : " << it->first;
                        ImGui::RadioButton(ss.str().c_str(), (int*)&displayOption.completeTaskHandle, it->first);
                    }
                }
                ImGui::Separator();
                if (!g_CompletedTasks.empty() && displayOption.completeTaskHandle != Invalid_Task_Handle)
                {
                    Task *task = g_CompletedTasks[displayOption.completeTaskHandle];
                    auto &temp = *(reinterpret_cast<RenderSetting*>(task->userData));
                    ImGui::BulletText("%s", temp.str().c_str());
                }
            ImGui::End();

            ImGui::Begin(profileWindowName, &displayOption.showSplitWindow);

                // fetch profiler data from currently picked completed task
                if (displayOption.showProfilerData && !g_CompletedTasks.empty() && displayOption.completeTaskHandle != Invalid_Task_Handle)
                {
                    Task *task = g_CompletedTasks[displayOption.completeTaskHandle];
                    ImGui::Text("%s", task->profilerData.c_str());
                }
            ImGui::End();

            ImGui::Begin(mainWindowName, &displayOption.showSplitWindow);
                if (!g_CompletedTasks.empty() && displayOption.completeTaskHandle != Invalid_Task_Handle)
                {
                    Task *task = g_CompletedTasks[displayOption.completeTaskHandle];
                    auto &temp = *(reinterpret_cast<RenderSetting*>(task->userData));
                    intptr_t renderedTexture = quadRender.Update(temp.data, (sizeof(float) * width * height * 3));
                    ImVec2 uv0(0, 0);
                    ImVec2 uv1(1, 1);
                    if (displayOption.flipVertical)
                    {
                        uv0 = ImVec2(0, 1);
                        uv1 = ImVec2(1, 0);
                    }
                    if (displayOption.fitToWindow)
                    {
                        ImVec2 size((float)width, (float)height);
                        const float scale = std::min(ImGui::GetContentRegionAvail().x / size.x, ImGui::GetContentRegionAvail().y / size.y);
                        size.x *= scale;
                        size.y *= scale;
                        ImGui::Image((ImTextureID)renderedTexture, size, uv0, uv1);
                    }
                    else
                    {
                        ImGui::Image((ImTextureID)renderedTexture, ImVec2(width,height), uv0, uv1);
                    }
                }
                else
                {
                    if (displayOption.fitToWindow)
                    {
                        ImVec2 size((float)width_, (float)height_);
                        const float scale = std::min(ImGui::GetContentRegionAvail().x / size.x, ImGui::GetContentRegionAvail().y / size.y);
                        size.x *= scale;
                        size.y *= scale;
                        ImGui::Image((ImTextureID)waitingTexture, size);
                    }
                    else
                    {
                        ImGui::Image((ImTextureID)waitingTexture, ImVec2(width_, height_));
                    }
                }
            ImGui::End();

            //< query newly scheduled task is done ?
            if (g_CompletedTasks.find(activeTaskHandle) == g_CompletedTasks.end() && TaskDone(activeTaskHandle))
            {
                {
                    // task is done, generate profiler data here
                    Task *task = TaskScheduler::GetScheduler()->QueryTask(activeTaskHandle);
                    auto ret = g_CompletedTasks.emplace(activeTaskHandle, task);
                    if (!ret.second)
                    {
                        Err("[Main Thread]: enque completed task : handle={} fails.", activeTaskHandle);
                    }
                    else
                        Log("[Main Thread]: enque completed task : handle={}.", activeTaskHandle);
                }

                activeTaskHandle = Invalid_Task_Handle;
            }
        }

        GUI::EndFrame();

        // glfw swap Front & Back Buffers
        glfwSwapBuffers(window);
        curTime  = glfwGetTime();
        lastTime = curTime;
        glfwPollEvents();
        update((float)(curTime - lastTime), window);
    }

    // Cleanup
    GUI::CleanUp();
    glfwTerminate();
}

//
// --eye 0 0.9 2.5 --dir 0 0.001 -1 --up 0 1 0 --fov 60
//
// --batch  --eye 0 0.9 2.5 --dir 0 0.001 -1 --up 0 1 0 --fov 60  .\scene\cornell_box.obj 
//
void main(int argc, char** argv)
{
    bool batchMode = false;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (!strcmp(argv[i], "--batch"))
            {
                batchMode = true;
            }
        }
    }

    utility::CPUProfiler::begin();
    if (batchMode)
    {
        Log("Batch Mode");
        EntryPointMain(argc, argv);
    }
    else
    {
        RenderSetting gSettings;
        GUIModeMain(gSettings);
    }
    utility::CPUProfiler::end();
}
