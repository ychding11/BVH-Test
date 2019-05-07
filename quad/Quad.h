#pragma once
#include "Config.h"

namespace Quad
{
    class Program;

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
}