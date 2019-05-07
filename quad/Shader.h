#pragma once

// third-party libraries
#include <string>

#include "Config.h"

namespace Quad
{
    class Shader
    {
    private:
        GLuint _object;
    public:
        Shader(const std::string& filePath, GLuint shaderType);
        GLuint object() const;
    };
}