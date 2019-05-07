#pragma once

#include "Quad.h"
#include "Program.h"
#include "interface.h"

#include <glm/glm.hpp>

namespace Quad
{
    Program *loadShaders(const std::string &vertex_shader_fileName, const std::string &frag_shader_fileName);

    enum RendererType
    {
        Renderer_Progressive,
        Renderer_Tiled,
    };

    struct RenderOptions
    {
        RenderOptions()
        {
            rendererType = Renderer_Tiled;
            maxSamples = 10;
            maxDepth = 2;
            numTilesX = 5;
            numTilesY = 5;
            useEnvMap = false;
            hdrMultiplier = 1.0f;
        }
        //std::string rendererType;
        int rendererType; // see RendererType
        int maxSamples;
        int maxDepth;
        int numTilesX;
        int numTilesY;
        bool useEnvMap;
        float hdrMultiplier;
    };

    // Use default Frame Buffer from glfw as color buffer
    class Renderer : public Observer
    {
    protected:
		GLuint _resultTexture;
		GLuint _presentTexture;
		Program *_quadShader; 
        bool _initialized;

        Quad *_quad;
        glm::ivec2 _screenSize;
        std::string _shadersDirectory;

    public:
		float invSamplesPerPixel;
		float* pixelData;

        Renderer(const std::string& shadersDirectory);
        virtual ~Renderer();
        virtual void render() ;

        const glm::ivec2 getScreenSize() const { return _screenSize; }
        void  setScreenSize(int x, int y) { _screenSize = glm::ivec2(x, y); }
        void  setScreenSize(const glm::ivec2& size)
        {
            _screenSize = size;
        }


    protected:
        virtual void init();
        virtual void deinit();

    protected:
        void copyTexture(GLuint srcTexId, GLuint dstTexId )
        {
            glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTexId, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0); // set copy source to GL_READ_BUFFER
            glBindTexture(GL_TEXTURE_2D, dstTexId); // set copy destination
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, _screenSize.x, _screenSize.y);
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 0, 0, _screenSize.x, _screenSize.y, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

    public:

        virtual void handleScreenSizeChange(const glm::ivec2 &newScreenSize) override
        {
            _screenSize = newScreenSize;
        }
	    virtual void handleNewRenderResult(void* newData, int newSize) override
        {
            assert(newData != nullptr && " NULL pointer !!!");
            assert(newSize == (_screenSize.x * _screenSize.y * 3 * sizeof(float)) && " size mismatch !!!");

		    //// Update render result texture ////
            glBindTexture(GL_TEXTURE_2D, _resultTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSize.x, _screenSize.y , 0, GL_RGB, GL_FLOAT, newData);
		    //// Update render result texture ////
        }

    };
}