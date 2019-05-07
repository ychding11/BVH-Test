#include "Config.h"
#include "Renderer.h"

namespace Quad
{
    Program *loadShaders(const std::string &vertex_shader_fileName, const std::string &frag_shader_fileName)
    {
        std::vector<Shader> shaders;
        shaders.push_back(Shader(vertex_shader_fileName, GL_VERTEX_SHADER));
        shaders.push_back(Shader(frag_shader_fileName, GL_FRAGMENT_SHADER));
        return new Program(shaders);
    }

    Renderer::Renderer(const std::string& shadersDirectory)
        : _resultTexture(0)
        , _initialized(false)
        , _screenSize(1280, 720)
        , _shadersDirectory(shadersDirectory)
        , invSamplesPerPixel(1.f)
    {
        this->init();
    }

    Renderer::~Renderer()
    {
        if (_initialized)
        {
            this->deinit();
        }
    }

    void Renderer::deinit()
    {
        if (!_initialized)
        {
            return;
        }

        glDeleteTextures(1, &_resultTexture);
        glDeleteTextures(1, &_presentTexture);
        
        delete _quad;
        delete _quadShader;

        _initialized = false;
    }

    void Renderer::init()
    {
        if (_initialized)
        {
            return;
        }

        _quad = new Quad();

		//----------------------------------------------------------
	    // Shaders
	    //----------------------------------------------------------
		_quadShader = loadShaders(_shadersDirectory + "quadVert.glsl", _shadersDirectory + "quadFrag.glsl");

		//----------------------------------------------------------
		// No Need to FBO Setup, use default one (draw on screen directly)
		//----------------------------------------------------------

		//----------------------------------------------------------
		// Shader Parameter Setup
		//----------------------------------------------------------
        //Create Texture for Result 
        glBindTexture(GL_TEXTURE_BUFFER, _resultTexture);
        glBindTexture(GL_TEXTURE_2D, _resultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSize.x, _screenSize.y , 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_BUFFER, _presentTexture);
        glBindTexture(GL_TEXTURE_2D, _presentTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSize.x, _screenSize.y , 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        _initialized = true;
    }

	void Renderer::render()
	{
		if (!_initialized)
		{
            assert(0 && "Call Render Before Init !!!");
			return;
		}

		//// Update Shader Parameter ////
		GLuint shaderObject;
		_quadShader->use();
		shaderObject = _quadShader->object();
		glUniform1i(glGetUniformLocation(shaderObject, "resultTexture"), 0);
		glUniform1f(glGetUniformLocation(shaderObject, "invSamplesPerPixel"), invSamplesPerPixel);
		_quadShader->stopUsing();
		//// Update Shader Parameter ////


        // https://stackoverflow.com/questions/11617013/why-would-glbindframebuffergl-framebuffer-0-result-in-blank-screen-in-cocos2d
        // FrameBuffer 0 = default frame buffer in glfw. But it maybe not true on iOS platform.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, _screenSize.x, _screenSize.y);

        // Bind Parameter
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _resultTexture);

		_quad->Draw(_quadShader);
	}
}