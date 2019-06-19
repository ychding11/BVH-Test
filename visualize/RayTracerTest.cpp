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

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "BVH.h"
#include "Sphere.h"
#include "interface.h"
#include "raytracer.h"
#include "Renderer.h"
#include "Stopwatch.h"

#include "smallpt.h"

std::vector<ProfilerEntry> CPUProfiler::ProfilerData(16);
std::vector<ProfilerEntry> CPUProfiler::ProfilerDataA;

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

int main()
{
	srand(unsigned int(time(0)));

	GLFWwindow *window;
	glfwInit();
	window = glfwCreateWindow(settings.screenSize.x, settings.screenSize.y, "smallpt", 0, 0);
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

    BVHTracer bvhTracer(settings.objectNum, settings.screenSize.x, settings.screenSize.y, msgStream);
    smallpt::smallptTest smallpter(settings.screenSize.x, settings.screenSize.y, settings.samples);
    Observer *uiObserver = &bvhTracer;
    uiObserverSmallpt = &smallpter;

	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
        FrameRateDetector frameratedetector;
        frameratedetector.start();
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");

        CPUProfiler::begin();
		{
            CPUProfiler profiler("ImGui");

			if (ImGui::Combo("Scene", &settings.testIndex, "bvhTest\0smallpt\0"))
			{
				//uiObserver->handleTestIndexChange(settings.testIndex);
			}
			if (settings.testIndex == 0)
			{
				if (ImGui::SliderInt("spp", &settings.samples, 1, 1024))
				{
					//uiObserverSmallpt->handleSampleCountChange(settings.samples);
				}
				if (ImGui::SliderFloat2("translate", &settings.focusOffset.x, -0.5f, 0.5f))
				{
					uiObserver->handleFocusOffsetChange(settings.focusOffset);
				}
				if (ImGui::SliderFloat("zoom", &settings.positionOffset, 0.0f, 10.f))
				{
					uiObserver->handlePositionOffsetChange(settings.positionOffset);
				}
			}
			else if (settings.testIndex == 1)
			{
				ImGui::BeginGroup();

				if (ImGui::Button("Save"))
				{
					stbi_write_png("output.png", settings.screenSize.x, settings.screenSize.y, 4, smallpter.getRenderResult(), settings.screenSize.x * 4);
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause"))
				{
				}

				uint32_t sceneMask = settings.sceneMask;
				if (ImGui::SliderFloat("IOR", &settings.ior, 1.0f, 2.2f))
				{
					uiObserverSmallpt->handleIORChange(settings.ior);
				}
				if (ImGui:: Checkbox("Sphere Scene", &settings.sphereScene))
				{
					if (settings.sphereScene)
					{
						sceneMask |= 0x1;
					}
					else
					{
						sceneMask &= ~0x1;
					}
				}
				ImGui::SameLine();
				if (ImGui:: Checkbox("Triangle Scene", &settings.triangleScene))
				{
					if (settings.triangleScene)
					{
						sceneMask |= 0x2;
					}
					else
					{
						sceneMask &= ~0x2;
					}
				}
				ImGui::ColorEdit4("clear color", gClearColor);
				ImGui::EndGroup();
				if (sceneMask != settings.sceneMask)
				{
					settings.sceneMask = sceneMask;
					uiObserverSmallpt->handleSceneMaskChange(settings.sceneMask);
				}
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
            CPUProfiler profiler("Test Running");
            if (settings.testIndex == 0)
            {
                bvhTracer.run();
                quadRender.handleNewRenderResult(bvhTracer.getRenderResult(), sizeInBytes);
            }
            else
            {
				smallpter.run();
				progress = smallpter.getRenderProgress();
                quadRender.handleNewRenderResult(smallpter.getRenderResult(), sizeInBytes);
            }
        }
		
        {
            CPUProfiler profiler("Quad Render");
            quadRender.render();
        }

        CPUProfiler::end();
        frameratedetector.stop();

        ImGui::BeginChild("Profiler&Log", ImVec2(0, 0), true);
		ImGui::Text("Frame time %.3f ms\t(%.1f FPS)", frameratedetector.frametime(), frameratedetector.framerate());
        ImGui::Text("%s", CPUProfiler::end().c_str());
        ImGui::Text("-------------------");
        ImGui::Text("%s", progress.c_str());
        ImGui::EndChild();

		ImGui::End();

		// ImGui Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw swap Front & Back Buffers
		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
    return 0;
}