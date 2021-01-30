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
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "BVH.h"
#include "interface.h"
#include "raytracer.h"
#include "Renderer.h"

std::vector<ProfilerEntry> CPUProfiler::ProfilerData(16);
std::vector<ProfilerEntry> CPUProfiler::ProfilerDataA;

struct Setting;
static Setting settings;

bool keyPressed = false;
void update(float secondsElapsed, GLFWwindow *window)
{
	keyPressed = false;
	//Camera Movement
	if (glfwGetKey(window, 'W')) {
		keyPressed = true;
	}
	else if (glfwGetKey(window, 'S')) {
		keyPressed = true;
	}
	if (glfwGetKey(window, 'A')) {
		keyPressed = true;
	}
	else if (glfwGetKey(window, 'D')) {
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
}

void main()
{
	srand(unsigned int(time(0)));

	GLFWwindow *window;
	glfwInit();
	window = glfwCreateWindow(settings.screenSize.x, settings.screenSize.y, "BVH-Lab", 0, 0);
	glfwSetWindowPos(window, 300, 100);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	glfwSetCursorPos(window, 0, 0);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0); // turn off vsync
	glewInit();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

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
    //quadRender.setScreenSize(settings.screenSize);

    std::ostringstream msgStream;

    BVHTracer bvhTracer(settings.objectNum, settings.screenSize.x, settings.screenSize.y, msgStream);
    Observer *uiObserver = &bvhTracer;

    std::string profileInfo;
	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

        GUI::BeginFrame();

        CPUProfiler::begin();
		{
            CPUProfiler profiler("imgui");
			ImGui::Begin("Settings");

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 33.33f, 1000.0f / 33.33f);
			if (ImGui::Combo("Scene", &settings.testIndex, "bvh\0noise\0"))
			{
				uiObserver->handleTestIndexChange(settings.testIndex);
			}
			if (ImGui::SliderInt("Objects number", &settings.objectNum, 0, 100))
			{
				uiObserver->handleObjectNumChange(settings.objectNum);
			}
			if (ImGui::SliderFloat2("focus offset", &settings.focusOffset.x, -0.5f, 0.5f))
			{
				uiObserver->handleFocusOffsetChange(settings.focusOffset);
			}
			if (ImGui::SliderFloat("position offset", &settings.positionOffset, 0.0f, 10.f))
			{
				uiObserver->handlePositionOffsetChange(settings.positionOffset);
			}

            ImGui::BulletText("Object Number %d \n", bvhTracer.objectNum());

            ImGui::Text("%s", profileInfo.c_str());

            std::string errorMessage;
            errorMessage = msgStream.str();

            if (!errorMessage.empty())
                ImGui::OpenPopup("Error");
            if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("%s", errorMessage.c_str());
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

			ImGui::End();
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, GL_TRUE);
		double presentTime = glfwGetTime();
		update((float)(presentTime - lastTime), window);
		lastTime = presentTime;

        {
            CPUProfiler profiler("Run Test");
            bvhTracer.run();
        }
		
        {
            CPUProfiler profiler("Quad Render");
            quadRender.Update(bvhTracer._pixels, (sizeof(float) * settings.screenSize.x * settings.screenSize.y * 3));
            quadRender.Render();
        }
        msgStream.str(""); //clear content in stream
        profileInfo = CPUProfiler::end();

        GUI::EndFrame();

		// glfw swap Front & Back Buffers
		glfwSwapBuffers(window);
	}

	// Cleanup
    GUI::CleanUp();
	glfwTerminate();
}
