#include "../../../include/odfaeg/Graphics/vbo.h"
#include "glCheck.h"
#include <iostream>
#include <string.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
namespace odfaeg {
    namespace graphic {
        VBO::VBO() {
            m_vbo = 0;
        }
        void VBO::bind() {
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
        }
        void VBO::unbind() {
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
        void VBO::create (const void* data, const unsigned int dataSize) {
            //ensureGlContext();
            if (m_vbo == 0) {
                GLuint vbo;
                glCheck(glGenBuffers(1, &vbo));
                m_vbo = static_cast<unsigned int>(vbo);

            }

            if (isAvailable() && m_vbo != 0 ) {
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
                glCheck(glBufferDataARB(GL_ARRAY_BUFFER_ARB,dataSize, data, GL_STREAM_DRAW));
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
            } else {
                std::cerr<<"Failed to create the vbo, you should check if vbo are avalaible first! (by calling the VBO::isAvailable function)"<<std::endl;
            }
        }
        bool VBO::isAvailable() {
            //ensureGlContext();
            // Make sure that GLEW is initialized
            priv::ensureGlewInit();
            return GLEW_ARB_vertex_buffer_object;
        }
        bool VBO::update(const void* data, const unsigned int dataSize) {
            if (isAvailable() && m_vbo != 0) {
                //ensureGlContext();
                GLvoid *pos_vbo = nullptr;
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
                pos_vbo = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                if (pos_vbo == nullptr)
                    return false;
                memcpy(pos_vbo,data, dataSize);
                glCheck(glUnmapBuffer(GL_VERTEX_ARRAY));
                pos_vbo = nullptr;
                glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
            }
            return true;
        }
        VBO::~VBO() {
            if (m_vbo) {
                //ensureGlContext();
                GLuint vbo = static_cast<GLuint>(m_vbo);
                glDeleteBuffers(1, &vbo);
            }
        }
    }
}
