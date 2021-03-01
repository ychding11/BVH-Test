#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <cassert> 

namespace Quad
{
    class Shader
    {
    private:
        GLuint _object;
    public:
        Shader(const std::string& filePath, GLenum shaderType);
        GLuint object() const;
    };

    Shader::Shader(const std::string& filePath, GLenum shaderType)
    {
        std::ifstream f;
        f.open(filePath.c_str(), std::ios::in | std::ios::binary);
        if (!f.is_open())
        {
            printf("Failed to open file: %s\n", filePath.c_str());
            return;
        }

        //read whole file into stringstream buffer
        std::stringstream buffer;
        buffer << f.rdbuf();

        std::string source = buffer.str();
        _object = glCreateShader(shaderType);
        const GLchar *src = (const GLchar *)source.c_str();
        glShaderSource(_object, 1, &src, 0);
        glCompileShader(_object);
        GLint success = 0;
        glGetShaderiv(_object, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            std::string msg("Error while compiling shader\n");
            GLint logSize = 0;
            glGetShaderiv(_object, GL_INFO_LOG_LENGTH, &logSize);
            char *info = new char[logSize + 1];
            glGetShaderInfoLog(_object, logSize, NULL, info);
            msg += info;
            delete[] info;
            glDeleteShader(_object);
            _object = 0;
            printf("Shader compilation error %s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }

    GLuint Shader::object() const
    {
        return _object;
    }
    class Program
    {
    private:
        GLuint _object;
    public:
        Program(const std::vector<Shader> shaders);
        ~Program()
        {
            glDeleteProgram(_object);
        }

        void use()
        {
            glUseProgram(_object);
        }

        void stopUsing()
        {
            glUseProgram(0);
        }

        GLuint object()
        {
            return _object;
        }
    };
    
    Program::Program(const std::vector<Shader> shaders)
    {
        _object = glCreateProgram();
        for (unsigned i = 0; i < shaders.size(); i++)
            glAttachShader(_object, shaders[i].object());

        glLinkProgram(_object);

        for (unsigned i = 0; i < shaders.size(); i++)
            glDetachShader(_object, shaders[i].object());
        GLint success = 0;
        glGetProgramiv(_object, GL_LINK_STATUS, &success);
        if (success == GL_FALSE)
        {
            std::string msg("Error while linking program\n");
            GLint logSize = 0;
            glGetProgramiv(_object, GL_INFO_LOG_LENGTH, &logSize);
            char *info = new char[logSize + 1];
            glGetShaderInfoLog(_object, logSize, NULL, info);
            msg += info;
            delete[] info;
            glDeleteProgram(_object);
            _object = 0;
            //std::cout << msg << std::endl; TODO
            throw std::runtime_error(msg);
        }
    }


    Program *loadShaders(const std::string &vertex_shader_fileName, const std::string &frag_shader_fileName)
    {
        std::vector<Shader> shaders;
        shaders.push_back(Shader(vertex_shader_fileName, GL_VERTEX_SHADER));
        shaders.push_back(Shader(frag_shader_fileName, GL_FRAGMENT_SHADER));
        return new Program(shaders);
    }

    class Quad
    {
    public:
        Quad();

        virtual ~Quad()
        {
            glDeleteBuffers(1, &vbo);
            glDeleteVertexArrays(1, &vao);
        }

        void Draw(Program *);
    private:
        GLuint vao, vbo;
    };

    Quad::Quad()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        //Vertex data
        float vertices[] =
        {
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));

        glBindVertexArray(0);
    }

    void Quad::Draw(Program *shader)
    {
        shader->use();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        shader->stopUsing();
    }
    Renderer::Renderer(std::string shadersDirectory)
        : _resultTexture(0)
        , _initialized(false)
        , _screenSizeX(1280)
        , _screenSizeY(720)
        , _shadersDirectory(shadersDirectory)
        , invSamplesPerPixel(1.f)
    {
        this->init();
    }

    Renderer::~Renderer()
    {
        if (_initialized)
        {
            glDeleteTextures(1, &_resultTexture);
        
            delete _quad;
            delete _quadShader;
        }
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
        glBindTexture(GL_TEXTURE_2D, _resultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSizeY, _screenSizeY , 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        _initialized = true;
    }

    GLuint Renderer::LoadTexture(std::string path, int &width, int &height)
    {
        return 0;
    }

    GLuint Renderer::CreateRenderTexture()
    {
		GLuint renderTexture;
        glGenTextures(1, &renderTexture);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSizeY, _screenSizeY , 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        return renderTexture;
    }

    void Renderer::UpdateRenderTexture(GLuint renderTexture, void* newData, int newSize)
    {
        assert(newData != nullptr && " NULL pointer !!!");
        assert(newSize == (_screenSizeX * _screenSizeY * 3 * sizeof(float)) && " Texture size mismatch !!!");

        //// Update render result texture ////
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSizeX, _screenSizeY, 0, GL_RGB, GL_FLOAT, newData);
    }

    GLuint Renderer::Update(void* newData, int newSize)
    {
        assert(newData != nullptr && " NULL pointer !!!");
        assert(newSize == (_screenSizeX * _screenSizeY * 3 * sizeof(float)) && " Texture size mismatch !!!");

		//// Update render result texture ////
        glBindTexture(GL_TEXTURE_2D, _resultTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, _screenSizeX, _screenSizeY , 0, GL_RGB, GL_FLOAT, newData);
		//// Update render result texture ////
        return _resultTexture;
    }

	void Renderer::Render()
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
		glViewport(0, 0, _screenSizeY, _screenSizeY);

        // Bind Parameter
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _resultTexture);

		_quad->Draw(_quadShader);
	}
}