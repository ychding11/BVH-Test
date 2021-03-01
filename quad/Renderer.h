#pragma once

#include <GL/glew.h>
#include <string>

namespace Quad
{
    class Quad;
    class Program;

    // Use default Frame Buffer from glfw as color buffer
    class Renderer 
    {
    protected:
		GLuint _resultTexture;
		Program *_quadShader; 
        bool _initialized;

        Quad *_quad;
        int _screenSizeX;
        int _screenSizeY;
        std::string _shadersDirectory;

    public:
		float invSamplesPerPixel;
		float* pixelData;

        Renderer(std::string shadersDirectory = "../quad/shaders/");
        virtual ~Renderer();

        void  setScreenSize(int x, int y) { _screenSizeX = x; _screenSizeY = y;  }

        GLuint Update(void* newData, int newSize);

        GLuint LoadTexture(std::string path, int &width, int &height);
        GLuint CreateRenderTexture();

        void UpdateRenderTexture(GLuint renderTexture, void* newData, int newSize);

        void Render();

    protected:
        void init();
        void copyTexture(GLuint srcTexId, GLuint dstTexId )
        {
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexId, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0); // set copy source to GL_READ_BUFFER
            glBindTexture(GL_TEXTURE_2D, dstTexId); // set copy destination
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _screenSizeX, _screenSizeY);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 0, 0, _screenSizeX, _screenSizeY, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    };
}