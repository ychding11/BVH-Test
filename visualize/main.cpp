#include <cstdio>
#include <vector>
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

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "BVH.h"
#include "interface.h"
#include "raytracer.h"
#include "Renderer.h"
#include "IconsFontAwesome4.h"

std::vector<ProfilerEntry> CPUProfiler::ProfilerData(16);
std::vector<ProfilerEntry> CPUProfiler::ProfilerDataA;

struct Setting;
static Setting settings;

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

namespace GUI
{
    // Start the Dear ImGui frame
    void BeginFrame(void)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    // Render the GUI element 
    void EndFrame(void)
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }


    // Setup Platform/Renderer bindings
    void Setup(GLFWwindow* window, const char* glsl_version)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsResizeFromEdges = true;
        io.DisplaySize.x = (float)w;
        io.DisplaySize.y = (float)h;
        io.IniFilename = nullptr;
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;


        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui::StyleColorsClassic();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    // Cleanup
    void CleanUp()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Dialog(const char *title, const char *msg)
    {
        ImGui::OpenPopup(title);
        if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s", msg);
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

namespace Logging
{
    static spdlog::logger *sLogger = nullptr;

    void Init(void)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::warn);
        //console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
        file_sink->set_level(spdlog::level::trace);

        sLogger = new spdlog::logger("multi_sink", { console_sink, file_sink });
        sLogger -> set_level(spdlog::level::debug);
    }

    spdlog::logger *Logger()
    {
        if (sLogger == nullptr)
        {
            Init();
        }
        return sLogger;
    }

}

int width = 1280;
int height = 720;
void main()
{
    Warn("this should appear in both console and file");
    Log("this message should is just a test");

    srand(unsigned int(time(0)));

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

    std::ostringstream msgStream;

    BVHTracer bvhTracer(settings.objectNum, width, height, msgStream);
    Observer *uiObserver = &bvhTracer;
    bool initDockLayout = true;
    std::string profileInfo;
    double lastTime = glfwGetTime();
    double curTime  = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        GUI::BeginFrame();

        CPUProfiler::begin();
        {
            CPUProfiler profiler("imgui");

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
            const char *testOptionsWindowName = ICON_FA_GLOBE "Test Options";
            const char *profileWindowName = ICON_FA_GLOBE "Profile Data";
            const char *statusWindowName = ICON_FA_GLOBE "Status";
            const char *mainWindowName = ICON_FA_GLOBE "Main";
            if (initDockLayout)
            {
                ImGuiID dockLeftId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.20f, nullptr, &dockSpaceId);
                ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Right, 0.30f, nullptr, &dockSpaceId);
                ImGuiID dockRightBottomId = ImGui::DockBuilderSplitNode(dockRightId, ImGuiDir_Down, 0.50f, nullptr, &dockRightId);
                ImGui::DockBuilderDockWindow(testOptionsWindowName, dockLeftId);
                ImGui::DockBuilderDockWindow(statusWindowName, dockRightId);
                ImGui::DockBuilderDockWindow(profileWindowName, dockRightBottomId);
                ImGui::DockBuilderDockWindow(mainWindowName, dockSpaceId);
                ImGui::DockBuilderFinish(dockSpaceId);
                initDockLayout = false;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
            if (ImGui::BeginMainMenuBar())
            {
                ImGui::PopStyleVar();
                if (ImGui::BeginMenu(ICON_FA_CUBE " Model"))
                {
                    if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open..."))
                    { }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_CAMERA " Camera"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_EYE " View"))
                {
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_WINDOWS " Settings"))
                {
                    if (ImGui::Combo("Scene", &settings.testIndex, "bvh\0noise\0"))
                    {
                        bvhTracer.handleTestIndexChange(settings.testIndex);
                    }
                    if (ImGui::SliderInt("Objects number", &settings.objectNum, 0, 100))
                    {
                        bvhTracer.handleObjectNumChange(settings.objectNum);
                    }
                    if (ImGui::SliderFloat2("focus offset", &settings.focusOffset.x, -0.5f, 0.5f))
                    {
                        bvhTracer.handleFocusOffsetChange(settings.focusOffset);
                    }
                    if (ImGui::SliderFloat("position offset", &settings.positionOffset, 0.0f, 10.f))
                    {
                        bvhTracer.handlePositionOffsetChange(settings.positionOffset);
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
            else
            {
                ImGui::PopStyleVar();
            }

            ImGui::End();

            bool showWindow = true;
            ImGui::Begin(testOptionsWindowName, &showWindow);
            ImGui::BulletText("Object Number %d \n", bvhTracer.objectNum());
            ImGui::End();

            if (!msgStream.str().empty())
                GUI::Dialog("Message", msgStream.str().c_str());

            ImGui::Begin(statusWindowName, &showWindow);
            ImGui::BulletText("Object Number %d \n", bvhTracer.objectNum());
            ImGui::BulletText("Application average %.3f ms/frame (%.1f FPS)", 33.33f, 1000.0f / 33.33f);
            ImGui::End();

            ImGui::Begin(profileWindowName, &showWindow);
            ImGui::Text("%s", profileInfo.c_str());
            ImGui::End();

            bvhTracer.run();
            intptr_t retTexture = quadRender.Update(bvhTracer._pixels, (sizeof(float) * width * height * 3));

            ImGui::Begin(mainWindowName, &showWindow);
            ImGui::Image((ImTextureID)retTexture, ImVec2(width,height));
            ImGui::End();
        }

        

        msgStream.str(""); //clear content in stream
        profileInfo = CPUProfiler::end();


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
