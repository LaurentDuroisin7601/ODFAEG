////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2013 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../../../include/odfaeg/Graphics/shader.h"
#include "../../../include/odfaeg/Graphics/texture.h"
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Err.hpp>
#include <fstream>
#include <vector>
#include <array>
#ifndef VULKAN
#include "glCheck.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#else
#include <shaderc/shaderc.hpp>
#include "../../../include/odfaeg/Window/vkSettup.hpp"
#endif // VULKAN
using namespace sf;

namespace
{
    #ifndef VULKAN
    // Retrieve the maximum number of texture units available
    GLint getMaxTextureUnits()
    {
        GLint maxUnits;
        glCheck(glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &maxUnits));
        return maxUnits;
    }
    #endif

    // Read the contents of a file into an array of char
    bool getFileContents(const std::string& filename, std::vector<char>& buffer)
    {
        std::ifstream file(filename.c_str(), std::ios_base::binary);
        if (file)
        {
            file.seekg(0, std::ios_base::end);
            std::streamsize size = file.tellg();
            if (size > 0)
            {
                file.seekg(0, std::ios_base::beg);
                buffer.resize(static_cast<std::size_t>(size));
                file.read(&buffer[0], size);
            }
            buffer.push_back('\0');
            return true;
        }
        else
        {
            return false;
        }
    }

    // Read the contents of a stream into an array of char
    bool getStreamContents(sf::InputStream& stream, std::vector<char>& buffer)
    {
        bool success = true;
        sf::Int64 size = stream.getSize();
        if (size > 0)
        {
            buffer.resize(static_cast<std::size_t>(size));
            stream.seek(0);
            sf::Int64 read = stream.read(&buffer[0], size);
            success = (read == size);
        }
        buffer.push_back('\0');
        return success;
    }
}


namespace odfaeg {
    namespace graphic {
        #ifdef VULKAN
        Shader::Shader() {
            vkSettup = nullptr;
        }
        void Shader::setVkSettup (window::VkSettup* vkSettup) {
            this->vkSettup = vkSettup;
        }
        ////////////////////////////////////////////////////////////
        bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
        {
            // Read the vertex shader file
            std::vector<char> vertexShader;
            if (!getFileContents(vertexShaderFilename, vertexShader))
            {
                err() << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
                return false;
            }

            // Read the fragment shader file
            std::vector<char> fragmentShader;
            if (!getFileContents(fragmentShaderFilename, fragmentShader))
            {
                err() << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0]);
        }
        ////////////////////////////////////////////////////////////
        bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader)
        {
            // Compile the shader program
            return compile(vertexShader.c_str(), fragmentShader.c_str());
        }
        ////////////////////////////////////////////////////////////
        bool Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream)
        {
            // Read the vertex shader code from the stream
            std::vector<char> vertexShader;
            if (!getStreamContents(vertexShaderStream, vertexShader))
            {
                err() << "Failed to read vertex shader from stream" << std::endl;
                return false;
            }

            // Read the fragment shader code from the stream
            std::vector<char> fragmentShader;
            if (!getStreamContents(fragmentShaderStream, fragmentShader))
            {
                err() << "Failed to read fragment shader from stream" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0]);
        }
        ////////////////////////////////////////////////////////////
        bool Shader::compile(const char* vertexShaderCode, const char* fragmentShaderCode) {
            shaderc::Compiler compiler;
            shaderc::CompileOptions options;
            options.SetOptimizationLevel(shaderc_optimization_level_size);
            shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(vertexShaderCode, shaderc_glsl_vertex_shader, "shader_src", options);
            if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                std::cerr << "Failed to compile vertex shader :  "<<module.GetErrorMessage();
                return false;
            } else {
                spvVertexShaderCode = {module.cbegin(), module.cend()};
            }
            module = compiler.CompileGlslToSpv(fragmentShaderCode, shaderc_glsl_fragment_shader, "shader_src", options);
            if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                std::cerr << "Failed to compile fragment shader :  "<<module.GetErrorMessage();
                return false;
            } else {
                spvFragmentShaderCode = {module.cbegin(), module.cend()};
            }
            return true;
        }
        void Shader::createShaderModules() {
            if (vkSettup == nullptr) {
                throw core::Erreur (0, "No vulkan setup set!", 1);
            }
            VkShaderModuleCreateInfo createVSInfo{};
            createVSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createVSInfo.codeSize = spvVertexShaderCode.size();
            createVSInfo.pCode = spvVertexShaderCode.data();
            if (vkCreateShaderModule(vkSettup->getDevice(), &createVSInfo, nullptr, &vertexShaderModule) != VK_SUCCESS) {
                throw core::Erreur (0, "Failed to create vertex shader module", 1);
            }
            VkShaderModuleCreateInfo createFSInfo{};
            createFSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createFSInfo.codeSize = spvFragmentShaderCode.size();
            createFSInfo.pCode = spvFragmentShaderCode.data();
            if (vkCreateShaderModule(vkSettup->getDevice(), &createFSInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS) {
                throw core::Erreur (0, "Failed to create fragment shader module", 1);
            }
        }
        void Shader::cleanupShaderModules() {
            if (vkSettup == nullptr) {
                throw core::Erreur (0, "No vulkan setup set!", 1);
            }
            vkDestroyShaderModule(vkSettup->getDevice(), vertexShaderModule, nullptr);
            vkDestroyShaderModule(vkSettup->getDevice(), fragmentShaderModule, nullptr);
        }
        VkShaderModule Shader::getVertexShaderModule() {
            return vertexShaderModule;
        }
        VkShaderModule Shader::getFragmentShaderModule() {
            return fragmentShaderModule;
        }
        Shader::~Shader() {
        }
        #else // VULKAN
        ////////////////////////////////////////////////////////////
        Shader::CurrentTextureType Shader::CurrentTexture;
        unsigned int Shader::shading_language_version_major = 0;
        unsigned int Shader::shading_language_version_minor = 0;
        ////////////////////////////////////////////////////////////
        Shader::Shader() :
        m_shaderProgram (0),
        m_currentTexture(-1),
        m_textures      (),
        m_params        (),
        m_vertexAttribs ()
        {
            shading_language_version_major = getVersionMajor();
            shading_language_version_minor = getVersionMinor();
        }
        ////////////////////////////////////////////////////////////
        Shader::~Shader()
        {
            //ensureGlContext();

            // Destroy effect program
            if (m_shaderProgram) {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3)
                    glCheck(glDeleteProgram(m_shaderProgram));
                else
                    glCheck(glDeleteObjectARB(m_shaderProgram));
            }
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromFile(const std::string& filename, Type type)
        {
            // Read the file
            std::vector<char> shader;
            if (!getFileContents(filename, shader))
            {
                err() << "Failed to open shader file \"" << filename << "\"" << std::endl;
                return false;
            }

            // Compile the shader program
            if (type == Vertex)
                return compile(&shader[0], NULL, NULL, NULL);
            else if (type == Fragment)
                return compile(NULL, &shader[0], NULL, NULL);
            else if (type == Geometry)
                return compile(NULL, NULL, &shader[0], NULL);
            else
                return compile(NULL,NULL,NULL,&shader[0]);
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
        {
            // Read the vertex shader file
            std::vector<char> vertexShader;
            if (!getFileContents(vertexShaderFilename, vertexShader))
            {
                err() << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
                return false;
            }

            // Read the fragment shader file
            std::vector<char> fragmentShader;
            if (!getFileContents(fragmentShaderFilename, fragmentShader))
            {
                err() << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0], NULL, NULL);
        }

        ////////////////////////////////////////////////////////////
        bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, const std::string& geometryShaderFilename)
        {
            // Read the vertex shader file
            std::vector<char> vertexShader;
            if (!getFileContents(vertexShaderFilename, vertexShader))
            {
                err() << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
                return false;
            }

            // Read the fragment shader file
            std::vector<char> fragmentShader;
            if (!getFileContents(fragmentShaderFilename, fragmentShader))
            {
                err() << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                return false;
            }

            // Read the fragment shader file
            std::vector<char> geometryShader;
            if (!getFileContents(geometryShaderFilename, geometryShader))
            {
                err() << "Failed to open geometry shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0], &geometryShader[0], NULL);
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromMemory(const std::string& shader, Type type)
        {
            // Compile the shader program
            if (type == Vertex)
                return compile(shader.c_str(), NULL, NULL, NULL);
            else if (type == Fragment)
                return compile(NULL, shader.c_str(), NULL, NULL);
            else if (type == Geometry)
                return compile(NULL, NULL, shader.c_str(), NULL);
            else
                return compile(NULL,NULL,NULL,shader.c_str());
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader)
        {
            // Compile the shader program
            return compile(vertexShader.c_str(), fragmentShader.c_str(), NULL, NULL);
        }

        ////////////////////////////////////////////////////////////
        bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader)
        {
            // Compile the shader program
            return compile(vertexShader.c_str(), fragmentShader.c_str(), geometryShader.c_str(), NULL);
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromStream(InputStream& stream, Type type)
        {
            // Read the shader code from the stream
            std::vector<char> shader;
            if (!getStreamContents(stream, shader))
            {
                err() << "Failed to read shader from stream" << std::endl;
                return false;
            }

            // Compile the shader program
            if (type == Vertex)
                return compile(&shader[0], NULL, NULL, NULL);
            else if (type == Fragment)
                return compile(NULL, &shader[0], NULL, NULL);
            else if (type == Geometry)
                return compile(NULL, NULL, &shader[0], NULL);
            else
                return compile(NULL, NULL, NULL, &shader[0]);
        }


        ////////////////////////////////////////////////////////////
        bool Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream)
        {
            // Read the vertex shader code from the stream
            std::vector<char> vertexShader;
            if (!getStreamContents(vertexShaderStream, vertexShader))
            {
                err() << "Failed to read vertex shader from stream" << std::endl;
                return false;
            }

            // Read the fragment shader code from the stream
            std::vector<char> fragmentShader;
            if (!getStreamContents(fragmentShaderStream, fragmentShader))
            {
                err() << "Failed to read fragment shader from stream" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0], NULL, NULL);
        }
        ////////////////////////////////////////////////////////////
        bool Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream, InputStream& geometryShaderStream)
        {
            // Read the vertex shader code from the stream
            std::vector<char> vertexShader;
            if (!getStreamContents(vertexShaderStream, vertexShader))
            {
                err() << "Failed to read vertex shader from stream" << std::endl;
                return false;
            }

            // Read the fragment shader code from the stream
            std::vector<char> fragmentShader;
            if (!getStreamContents(fragmentShaderStream, fragmentShader))
            {
                err() << "Failed to read fragment shader from stream" << std::endl;
                return false;
            }

            std::vector<char> geometryShader;
            if (!getStreamContents(geometryShaderStream, geometryShader))
            {
                err() << "Failed to read geometry shader from stream" << std::endl;
                return false;
            }

            // Compile the shader program
            return compile(&vertexShader[0], &fragmentShader[0], &geometryShader[0], NULL);
        }
        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, unsigned int x)
        {
            if (m_shaderProgram)
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                    if (location != -1) {
                        glCheck(glUniform1ui(location, x));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1)
                        glCheck(glUniform1ui(location, x));

                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }
        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, float x)
        {
            if (m_shaderProgram)
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                    if (location != -1) {
                        glCheck(glUniform1f(location, x));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1)
                        glCheck(glUniform1fARB(location, x));

                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, float x, float y)
        {
            if (m_shaderProgram)
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                    if (location != -1) {
                        glCheck(glUniform2f(location, x, y));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1)
                        glCheck(glUniform2fARB(location, x, y));

                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, float x, float y, float z)
        {
            if (m_shaderProgram)
            {
                 if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = getParamLocation(name);
                    if (location != -1) {
                        glCheck(glUniform3f(location, x, y, z));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1)
                        glCheck(glUniform3fARB(location, x, y, z));

                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, float x, float y, float z, float w)
        {
            if (m_shaderProgram)
            {


                 if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                    if (location != -1) {
                        glCheck(glUniform4f(location, x, y, z, w));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1)
                        glCheck(glUniform4fARB(location, x, y, z, w));

                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, const Vector2f& v)
        {
            setParameter(name, v.x, v.y);
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, const Vector3f& v)
        {
            setParameter(name, v.x, v.y, v.z);
        }

        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, const Color& color)
        {
            setParameter(name, color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
        }


        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, math::Matrix4f matrix)
        {
            if (m_shaderProgram)
            {

                 if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = getParamLocation(name);
                    if (location != -1) {
                        glCheck(glUniformMatrix4fv(location, 1, GL_FALSE,matrix.toGlMatrix().data()));
                    }
                    glCheck(glUseProgram(program));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1) {                        
                        glCheck(glUniformMatrix4fvARB(location, 1, GL_FALSE, matrix.toGlMatrix().data()));
                    }
                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }
        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, std::vector<math::Matrix4f> mats)
        {
            if (m_shaderProgram)
            {


                 if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    glCheck(glUseProgram(m_shaderProgram));
                    GLint location = getParamLocation(name);
                    if (location != -1) {
                        std::vector<float> matrices;
                        for (unsigned int i = 0; i < mats.size(); i++) {
                            std::array<float, 16> glmatrix = mats[i].toGlMatrix();
                            for (unsigned int j = 0; j < glmatrix.size(); j++) {
                                //std::cout<<"matrix : "<<i<<" element : "<<j<<" : "<<glmatrix[j]<<std::endl;
                                matrices.push_back(glmatrix[j]);
                            }
                        }
                        glCheck(glUniformMatrix4fv(location, static_cast<GLsizei>(mats.size()), GL_FALSE,matrices.data()));
                    }
                    glCheck(glUseProgram(0));
                } else {
                    // Enable program
                    GLhandleARB program = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
                    glCheck(glUseProgramObjectARB(m_shaderProgram));

                    // Get parameter location and assign it new values
                    GLint location = getParamLocation(name);
                    if (location != -1) {
                        std::vector<float> matrices;
                        for (unsigned int i = 0; i < mats.size(); i++) {
                            std::array<float, 16> glmatrix = mats[i].toGlMatrix();
                            for (unsigned int j = 0; j < glmatrix.size(); j++) {
                                matrices.push_back(glmatrix[j]);
                            }
                        }
                        glCheck(glUniformMatrix4fvARB(location, static_cast<GLsizei>(mats.size()), GL_FALSE, matrices.data()));
                    }
                    // Disable program
                    glCheck(glUseProgramObjectARB(program));
                }
            }
        }

        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, const Texture& texture)
        {
            if (m_shaderProgram)
            {


                // Find the location of the variable in the shader
                int location = getParamLocation(name);
                if (location != -1)
                {
                    // Store the location -> texture mapping
                    TextureTable::iterator it = m_textures.find(location);
                    if (it == m_textures.end())
                    {
                        // New entry, make sure there are enough texture units
                        static const GLint maxUnits = getMaxTextureUnits();
                        if (m_textures.size() + 1 >= static_cast<std::size_t>(maxUnits))
                        {
                            err() << "Impossible to use texture \"" << name << "\" for shader: all available texture units are used" << std::endl;
                            return;
                        }

                        m_textures[location] = &texture;
                    }
                    else
                    {
                        // Location already used, just replace the texture
                        it->second = &texture;
                    }
                }
            }
        }
        void Shader::setParameter(const std::string& name) {
            GLuint imageLoc = glGetUniformLocation(m_shaderProgram, name.c_str());
        }
        ////////////////////////////////////////////////////////////
        void Shader::bindAttribute(int location, const std::string& name) {
            if (m_shaderProgram) {

                GLint n;
                glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &n);
                if (location < n) {
                    if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                        glCheck(glBindAttribLocation(m_shaderProgram,location,name.c_str()));
                    } else {
                        glCheck(glBindAttribLocationARB(m_shaderProgram,location,name.c_str()));
                    }
                    m_currentAttrib = getVertexAttribLocation(name);
                } else {
                    err() << "Invalid attribute location " << location << " in vertex." << std::endl;
                }
            }
        }

        ////////////////////////////////////////////////////////////
        void Shader::setParameter(const std::string& name, CurrentTextureType)
        {
            if (m_shaderProgram)
            {


                // Find the location of the variable in the shader
                m_currentTexture = getParamLocation(name);
            }
        }


        ////////////////////////////////////////////////////////////
        void Shader::bind(const Shader* shader)
        {


            if (shader && shader->m_shaderProgram)
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    glCheck(glUseProgram(shader->m_shaderProgram));
                    shader->bindTextures();
                    // Bind the current texture
                    if (shader->m_currentTexture != -1)
                        glCheck(glUniform1i(shader->m_currentTexture, 0));

                } else {
                    // Enable the program
                    glCheck(glUseProgramObjectARB(shader->m_shaderProgram));

                    // Bind the textures
                    shader->bindTextures();

                    // Bind the current texture
                    if (shader->m_currentTexture != -1)
                        glCheck(glUniform1iARB(shader->m_currentTexture, 0));
                }
            }
            else
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    glCheck(glUseProgram(0));
                } else {
                    // Bind no shader
                    glCheck(glUseProgramObjectARB(0));
                }
            }
        }


        ////////////////////////////////////////////////////////////
        bool Shader::isAvailable()
        {


            // Make sure that GLEW is initialized
            priv::ensureGlewInit();
            /*if (shading_language_version_major >= 3 && shading_language_version_minor >= 3)
                return true;*/
            return true;
        }


        ////////////////////////////////////////////////////////////
        bool Shader::compile(const char* vertexShaderCode, const char* fragmentShaderCode, const char* geometryShaderCode, const char* computeShaderCode)
        {


            // First make sure that we can use shaders
            if (!isAvailable())
            {
                err() << "Failed to create a shader: your system doesn't support shaders "
                      << "(you should test Shader::isAvailable() before trying to use the Shader class)" << std::endl;
                return false;
            }

            // Destroy the shader if it was already created
            if (m_shaderProgram) {
                if (shading_language_version_major >= 3 && shading_language_version_minor >= 3)
                    glCheck(glDeleteProgram(m_shaderProgram));
                else
                    glCheck(glDeleteObjectARB(m_shaderProgram));
            }

            // Reset the internal state
            m_currentTexture = -1;
            m_textures.clear();
            m_params.clear();

            // Create the program
            if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3)
                m_shaderProgram = glCreateProgram();
            else
                m_shaderProgram = glCreateProgramObjectARB();
            if (computeShaderCode) {
                GLuint computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
                glCheck(glShaderSource(computeShaderID, 1, &computeShaderCode, NULL));
                glCheck(glCompileShader(computeShaderID));
                GLint success;
                glCheck(glGetShaderiv(computeShaderID, GL_COMPILE_STATUS,&success));
                if (success == GL_FALSE) {
                    int infoLogLength=0;
                    glCheck(glGetShaderiv(computeShaderID, GL_INFO_LOG_LENGTH, &infoLogLength));
                    std::vector<char> log;
                    log.resize(infoLogLength);
                    glCheck(glGetShaderInfoLog(computeShaderID, infoLogLength, 0, &log[0]));
                    std::cerr << "Failed to compile compute shader:" << std::endl                    
                    << std::string (log.begin(), log.end()) << std::endl;
                    glCheck(glDeleteShader(computeShaderID));
                    glCheck(glDeleteProgram(m_shaderProgram));
                }
                glCheck(glAttachShader(m_shaderProgram, computeShaderID));
                glCheck(glDeleteShader(computeShaderID));
            }
            // Create the vertex shader if needed
            if (vertexShaderCode)
            {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
                    glCheck(glShaderSource(vertexShaderID, 1, &vertexShaderCode, nullptr));
                    glCheck(glCompileShader(vertexShaderID));
                    GLint success;
                    glCheck(glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS,&success));
                    if (success == GL_FALSE) {
                        int infoLogLength;
                        glCheck(glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength));
                        std::vector<char> log;
                        log.resize(infoLogLength);
                        glCheck(glGetShaderInfoLog(vertexShaderID, infoLogLength, 0, &log[0]));
                        std::cerr << "Failed to compile vertex shader:" << std::endl
                            << std::string(log.begin(), log.end()) << std::endl;
                        glCheck(glDeleteShader(vertexShaderID));
                        glCheck(glDeleteProgram(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachShader(m_shaderProgram, vertexShaderID));
                    glCheck(glDeleteShader(vertexShaderID));

                } else {
                    // Create and compile the shader
                    GLhandleARB vertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
                    glCheck(glShaderSourceARB(vertexShader, 1, &vertexShaderCode, NULL));
                    glCheck(glCompileShaderARB(vertexShader));

                    // Check the compile log
                    GLint success;
                    glCheck(glGetObjectParameterivARB(vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &success));
                    if (success == GL_FALSE)
                    {
                        char log[1024];
                        glCheck(glGetInfoLogARB(vertexShader, sizeof(log), 0, log));
                        std::cerr << "Failed to compile vertex shader:" << std::endl
                              << log << std::endl;
                        glCheck(glDeleteObjectARB(vertexShader));
                        glCheck(glDeleteObjectARB(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachObjectARB(m_shaderProgram, vertexShader));
                    glCheck(glDeleteObjectARB(vertexShader));
                }
            }

            // Create the fragment shader if needed
            if (fragmentShaderCode)
            {
                // Create and compile the shader
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
                    glCheck(glShaderSource(fragmentShaderID, 1, &fragmentShaderCode, nullptr));
                    glCheck(glCompileShader(fragmentShaderID));
                    GLint success;
                    glCheck(glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS,&success));
                    if (success == GL_FALSE) {
                        int infoLogLength;
                        glCheck(glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength));
                        std::vector<char> log;
                        log.resize(infoLogLength);
                        glCheck(glGetShaderInfoLog(fragmentShaderID, infoLogLength, 0, &log[0]));
                        std::cerr << "Failed to compile fragment shader:" << std::endl
                            << std::string(log.begin(), log.end()) << std::endl;
                        glCheck(glDeleteShader(fragmentShaderID));
                        glCheck(glDeleteProgram(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachShader(m_shaderProgram, fragmentShaderID));
                    glCheck(glDeleteShader(fragmentShaderID));
                } else {
                    GLhandleARB fragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
                    glCheck(glShaderSourceARB(fragmentShader, 1, &fragmentShaderCode, NULL));
                    glCheck(glCompileShaderARB(fragmentShader));

                    // Check the compile log
                    GLint success;
                    glCheck(glGetObjectParameterivARB(fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &success));
                    if (success == GL_FALSE)
                    {
                        char log[1024];
                        glCheck(glGetInfoLogARB(fragmentShader, sizeof(log), 0, log));
                        std::cerr << "Failed to compile fragment shader:" << std::endl
                              << log << std::endl;
                        glCheck(glDeleteObjectARB(fragmentShader));
                        glCheck(glDeleteObjectARB(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachObjectARB(m_shaderProgram, fragmentShader));
                    glCheck(glDeleteObjectARB(fragmentShader));
                }
            }
            if (geometryShaderCode) {
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
                    glCheck(glShaderSource(geometryShaderID, 1, &geometryShaderCode, nullptr));
                    glCheck(glCompileShader(geometryShaderID));
                    GLint success;
                    glCheck(glGetShaderiv(geometryShaderID, GL_COMPILE_STATUS,&success));
                    if (success == GL_FALSE) {
                        int infoLogLength;
                        glCheck(glGetShaderiv(geometryShaderID, GL_INFO_LOG_LENGTH, &infoLogLength));
                        std::vector<char> log;
                        log.resize(infoLogLength);
                        glCheck(glGetShaderInfoLog(geometryShaderID, infoLogLength, 0, &log[0]));
                        std::cerr << "Failed to compile compute shader:" << std::endl
                            << std::string(log.begin(), log.end()) << std::endl;
                        glCheck(glDeleteShader(geometryShaderID));
                        glCheck(glDeleteProgram(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachShader(m_shaderProgram, geometryShaderID));
                    glCheck(glDeleteShader(geometryShaderID));
                } else {
                    GLhandleARB geometryShader = glCreateShaderObjectARB(GL_GEOMETRY_SHADER_ARB);
                    glCheck(glShaderSourceARB(geometryShader, 1, &geometryShaderCode, NULL));
                    glCheck(glCompileShaderARB(geometryShader));

                    // Check the compile log
                    GLint success;
                    glCheck(glGetObjectParameterivARB(geometryShader, GL_OBJECT_COMPILE_STATUS_ARB, &success));
                    if (success == GL_FALSE)
                    {
                        char log[1024];
                        glCheck(glGetInfoLogARB(geometryShader, sizeof(log), 0, log));
                        std::cerr << "Failed to compile fragment shader:" << std::endl
                              << log << std::endl;
                        glCheck(glDeleteObjectARB(geometryShader));
                        glCheck(glDeleteObjectARB(m_shaderProgram));
                        m_shaderProgram = 0;
                        return false;
                    }
                    glCheck(glAttachObjectARB(m_shaderProgram, geometryShader));
                    glCheck(glDeleteObjectARB(geometryShader));
                }
            }
            if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                glCheck(glLinkProgram(m_shaderProgram));
                GLint success;
                glCheck(glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success));
                if (success == GL_FALSE) {
                    int infoLogLength;
                    glCheck(glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength));
                    std::vector<char> log;
                    log.resize(infoLogLength);
                    glCheck(glGetProgramInfoLog(m_shaderProgram, infoLogLength, 0, &log[0]));
                    std::cerr << "Failed to link shader:" << std::endl
                        << std::string(log.begin(), log.end()) << std::endl;
                    glCheck(glDeleteProgram(m_shaderProgram));
                    m_shaderProgram = 0;
                    return false;
                }

            } else {
                // Link the program
                glCheck(glLinkProgramARB(m_shaderProgram));

                // Check the link log
                GLint success;
                glCheck(glGetObjectParameterivARB(m_shaderProgram, GL_OBJECT_LINK_STATUS_ARB, &success));
                if (success == GL_FALSE)
                {
                    char log[1024];
                    glCheck(glGetInfoLogARB(m_shaderProgram, sizeof(log), 0, log));
                    err() << "Failed to link shader:" << std::endl
                          << log << std::endl;
                    glCheck(glDeleteObjectARB(m_shaderProgram));
                    m_shaderProgram = 0;
                    return false;
                }
            }
            // Force an OpenGL flush, so that the shader will appear updated
            // in all contexts immediately (solves problems in multi-threaded apps)
            glCheck(glFlush());

            return true;
        }


        ////////////////////////////////////////////////////////////
        void Shader::bindTextures() const
        {
            TextureTable::const_iterator it = m_textures.begin();
            for (std::size_t i = 0; i < m_textures.size(); ++i)
            {
                GLint index = static_cast<GLsizei>(i + 1);
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    glCheck(glUniform1i(it->first, index));
                    glCheck(glActiveTexture(GL_TEXTURE0 + index));
                } else {
                    glCheck(glUniform1iARB(it->first, index));
                    glCheck(glActiveTextureARB(GL_TEXTURE0_ARB + index));
                }
                Texture::bind(it->second);
                ++it;
            }
            if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                glCheck(glActiveTexture(GL_TEXTURE0));
            } else {
                // Make sure that the texture unit which is left active is the number 0
                glCheck(glActiveTextureARB(GL_TEXTURE0_ARB));
            }
        }
        int Shader::getVertexAttribLocation(const std::string& name) {
            VertexAttribTable::const_iterator it = m_vertexAttribs.find(name);
            if (it != m_vertexAttribs.end()) {
                return it->second;
            } else {
                int location;
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    location = glGetAttribLocation(m_shaderProgram,name.c_str());
                } else {
                    location = glGetAttribLocationARB(m_shaderProgram,name.c_str());
                }
                if (location != -1)
                {
                    // Location found: add it to the cache
                    m_vertexAttribs.insert(std::make_pair(name, location));
                }
                else
                {
                    // Error: location not found
                    err() << "Vertex attrib " << name << "  not found in shader" << std::endl;
                }

                return location;
            }
        }

        ////////////////////////////////////////////////////////////
        int Shader::getParamLocation(const std::string& name)
        {
            // Check the cache
            ParamTable::const_iterator it = m_params.find(name);
            if (it != m_params.end())
            {
                // Already in cache, return it
                return it->second;
            }
            else
            {
                // Not in cache, request the location from OpenGL
                int location;
                if (shading_language_version_major > 3 || shading_language_version_major == 3 && shading_language_version_minor >= 3) {
                    location = glGetUniformLocation(m_shaderProgram, name.c_str());
                } else {
                    location = glGetUniformLocationARB(m_shaderProgram, name.c_str());
                }
                if (location != -1)
                {
                    // Location found: add it to the cache
                    m_params.insert(std::make_pair(name, location));
                }
                else
                {
                    // Error: location not found
                    err() << "Parameter " << name << " not found in shader" << std::endl;
                }

                return location;
            }
        }
        unsigned int Shader::getHandle() {
            return m_shaderProgram;
        }
        unsigned int Shader::getVersionMajor() {
            //ensureGlContext();
            const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
            if (glslversion)
                return glslversion[0] - '0';
            return 2;
        }
        unsigned int Shader::getVersionMinor() {
            //ensureGlContext();
            const GLubyte* glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
            if (glslversion)
                return glslversion[1] - '0';
            return 0;
        }
        #endif
    }
} // namespace sf
