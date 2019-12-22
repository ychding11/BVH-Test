#include <cstdio>
#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include <math.h>


// third-party libraries
#include <glog/logging.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>	
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "interface.h"
#include "Renderer.h"

#include "smallpt.h"
#include "profiler.h"

struct Setting;
static Setting settings;

float gClearColor[4] = {0, 0, 0, 1};

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

Observer *uiObserverSmallpt = nullptr;
Observer *uiObserverQuad= nullptr;

//! Handle back frame buffer size change
void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
    if (uiObserverSmallpt) uiObserverSmallpt->handleScreenSizeChange(glm::ivec2(width, height));
    if (uiObserverQuad) uiObserverQuad->handleScreenSizeChange(glm::ivec2(width, height));
    settings.screenSize.x = width; settings.screenSize.y = height;
}


static inline void BeginFrameImGUI(void)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

static inline void EndFrameImGUI(void)
{
	// ImGui Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static inline void DrawSettingsImGUI(void)
{
	ImGui::Begin("Settings");

	mei::CPUProfiler::begin();
	{
		mei::CPUProfiler profiler("ImGui");
		{
			ImGui::BeginGroup();

			ImGui::Button("Save");
			ImGui::SameLine();
			ImGui::Button("Pause");
			ImGui::SameLine();
			ImGui::ColorEdit4("clear color", gClearColor);

			ImGui::EndGroup();
		}
	}
	ImGui::End();
}

static inline void DrawLogsImGUI(const std::string &report)
{
	ImGui::Begin("Logs");

	ImGui::BeginChild("Profiler&Log", ImVec2(0, 0), true);
	ImGui::Text("%s", report.c_str());
	ImGui::EndChild();

	ImGui::End();
}

int main()
{
	srand(unsigned int(time(0)));

	GLFWwindow *window;
	glfwInit();
	window = glfwCreateWindow(settings.screenSize.x, settings.screenSize.y, "mei", 0, 0);
	glfwSetWindowPos(window, 300, 100);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetCursorPos(window, 0, 0);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

	glewInit();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

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
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

    Quad::Renderer quadRender("../quad/shaders/");
    quadRender.setScreenSize(settings.screenSize);
    uiObserverQuad = &quadRender;

    std::ostringstream msgStream;

    mei::smallptTest smallpter(settings.screenSize.x, settings.screenSize.y, settings.samples);
    uiObserverSmallpt = &smallpter;

	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
        mei::FrameRateDetector frameratedetector;
        frameratedetector.start();
		glfwPollEvents();

		BeginFrameImGUI();

		ImGui::Begin("Settings");

        mei::CPUProfiler::begin();
		{
            mei::CPUProfiler profiler("ImGui");

			ImGui::Combo("Scene", &settings.testIndex, "bvhTest\0smallpt\0");
			{
				ImGui::BeginGroup();

                ImGui::Button("Save");
				ImGui::SameLine();
				ImGui::Button("Pause");
				ImGui::SameLine();
				ImGui::ColorEdit4("clear color", gClearColor);
                
				ImGui::EndGroup();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, GL_TRUE);
		double presentTime = glfwGetTime();
		update((float)(presentTime - lastTime), window);
		lastTime = presentTime;

		std::string progress;
		int sizeInBytes = (sizeof(float) * settings.screenSize.x * settings.screenSize.y * 3);
        {
            mei::CPUProfiler profiler("Test Running");
			smallpter.run();
			progress = smallpter.getRenderProgress();
            quadRender.handleNewRenderResult(smallpter.getRenderResult(), sizeInBytes);
        }
        {
            mei::CPUProfiler profiler("Quad Render");
            quadRender.render();
        }
        
        frameratedetector.stop();

		msgStream.clear();
		msgStream << "FPS:" << frameratedetector.framerate() << "\n";
		msgStream << mei::CPUProfiler::end().c_str();
		msgStream << progress.c_str();
		DrawLogsImGUI(msgStream.str());
        
		EndFrameImGUI();

		glfwSwapBuffers(window); // glfw swap Front & Back Buffers
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
    return 0;
}