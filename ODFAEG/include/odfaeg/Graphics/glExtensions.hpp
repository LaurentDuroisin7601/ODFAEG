#ifndef ODFAEG_GLEXTENSIONS_HPP
#define ODFAEG_GLEXTENSIONS_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../config.hpp"

#ifdef ODFAEG_OPENGL_ES

    #include <ODFAEG/OpenGL.hpp>

    // ODFAEG requires at a bare minimum OpenGL ES 1.0 capability
    // Some extensions only incorporated by 2.0 are also required
    // OpenGL ES 1.0 is defined relative to OpenGL 1.3
    // OpenGL ES 1.1 is defined relative to OpenGL 1.5
    // OpenGL ES 2.0 is defined relative to OpenGL 2.0
    // All functionality beyond that is optional
    // and has to be checked for prior to use

    // Core since 1.0
    #define GLEXT_multitexture                        true
    #define GLEXT_texture_edge_clamp                  true
    #define GLEXT_EXT_texture_edge_clamp              true
    #define GLEXT_blend_minmax                        true
    #define GLEXT_glClientActiveTexture               glClientActiveTexture
    #define GLEXT_glActiveTexture                     glActiveTexture
    #define GLEXT_GL_TEXTURE0                         GL_TEXTURE0
    #define GLEXT_GL_CLAMP                            GL_CLAMP_TO_EDGE
    #define GLEXT_GL_CLAMP_TO_EDGE                    GL_CLAMP_TO_EDGE

    // Core since 1.1
    // 1.1 does not support GL_STREAM_DRAW so we just define it to GL_DYNAMIC_DRAW
    #define GLEXT_vertex_buffer_object                true
    #define GLEXT_GL_ARRAY_BUFFER                     GL_ARRAY_BUFFER
    #define GLEXT_GL_DYNAMIC_DRAW                     GL_DYNAMIC_DRAW
    #define GLEXT_GL_STATIC_DRAW                      GL_STATIC_DRAW
    #define GLEXT_GL_STREAM_DRAW                      GL_DYNAMIC_DRAW
    #define GLEXT_glBindBuffer                        glBindBuffer
    #define GLEXT_glBufferData                        glBufferData
    #define GLEXT_glBufferSubData                     glBufferSubData
    #define GLEXT_glDeleteBuffers                     glDeleteBuffers
    #define GLEXT_glGenBuffers                        glGenBuffers

    // The following extensions are listed chronologically
    // Extension macro first, followed by tokens then
    // functions according to the corresponding specification

    // The following extensions are required.

    // Core since 2.0 - OES_blend_subtract
    #define GLEXT_blend_subtract                      GL_OES_blend_subtract
    #define GLEXT_glBlendEquation                     glBlendEquationOES
    #define GLEXT_GL_FUNC_ADD                         GL_FUNC_ADD_OES
    #define GLEXT_GL_FUNC_SUBTRACT                    GL_FUNC_SUBTRACT_OES
    #define GLEXT_GL_FUNC_REVERSE_SUBTRACT            GL_FUNC_REVERSE_SUBTRACT_OES

    // The following extensions are optional.

    // Core since 2.0 - OES_blend_func_separate
    #ifdef ODFAEG_SYSTEM_ANDROID
        // Hack to make transparency working on some Android devices
        #define GLEXT_blend_func_separate                 false
    #else
        #define GLEXT_blend_func_separate                 GL_OES_blend_func_separate
    #endif
    #define GLEXT_glBlendFuncSeparate                 glBlendFuncSeparateOES

    // Core since 2.0 - OES_blend_equation_separate
    #ifdef ODFAEG_SYSTEM_ANDROID
        // Hack to make transparency working on some Android devices
        #define GLEXT_blend_equation_separate             false
    #else
        #define GLEXT_blend_equation_separate             GL_OES_blend_equation_separate
    #endif
    #define GLEXT_glBlendEquationSeparate             glBlendEquationSeparateOES

    // Core since 2.0 - OES_texture_npot
    #define GLEXT_texture_non_power_of_two            false

    // Core since 2.0 - OES_framebuffer_object
    #define GLEXT_framebuffer_object                  GL_OES_framebuffer_object
    #define GLEXT_glBindRenderbuffer                  glBindRenderbufferOES
    #define GLEXT_glDeleteRenderbuffers               glDeleteRenderbuffersOES
    #define GLEXT_glGenRenderbuffers                  glGenRenderbuffersOES
    #define GLEXT_glRenderbufferStorage               glRenderbufferStorageOES
    #define GLEXT_glBindFramebuffer                   glBindFramebufferOES
    #define GLEXT_glDeleteFramebuffers                glDeleteFramebuffersOES
    #define GLEXT_glGenFramebuffers                   glGenFramebuffersOES
    #define GLEXT_glCheckFramebufferStatus            glCheckFramebufferStatusOES
    #define GLEXT_glFramebufferTexture2D              glFramebufferTexture2DOES
    #define GLEXT_glFramebufferRenderbuffer           glFramebufferRenderbufferOES
    #define GLEXT_glGenerateMipmap                    glGenerateMipmapOES
    #define GLEXT_GL_FRAMEBUFFER                      GL_FRAMEBUFFER_OES
    #define GLEXT_GL_RENDERBUFFER                     GL_RENDERBUFFER_OES
    #define GLEXT_GL_DEPTH_COMPONENT                  GL_DEPTH_COMPONENT16_OES
    #define GLEXT_GL_COLOR_ATTACHMENT0                GL_COLOR_ATTACHMENT0_OES
    #define GLEXT_GL_DEPTH_ATTACHMENT                 GL_DEPTH_ATTACHMENT_OES
    #define GLEXT_GL_FRAMEBUFFER_COMPLETE             GL_FRAMEBUFFER_COMPLETE_OES
    #define GLEXT_GL_FRAMEBUFFER_BINDING              GL_FRAMEBUFFER_BINDING_OES
    #define GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION    GL_INVALID_FRAMEBUFFER_OPERATION_OES

    // Core since 3.0
    #define GLEXT_packed_depth_stencil                false

    // Core since 3.0
    #define GLEXT_framebuffer_blit                    false

    // Core since 3.0
    #define GLEXT_framebuffer_multisample             false

    // Core since 3.0 - NV_copy_buffer
    #define GLEXT_copy_buffer                         false

    // Core since 3.0 - EXT_sRGB
    #ifdef GL_EXT_sRGB
        #define GLEXT_texture_sRGB                        GL_EXT_sRGB
        #define GLEXT_GL_SRGB8_ALPHA8                     GL_SRGB8_ALPHA8_EXT
    #else
        #define GLEXT_texture_sRGB                        false
        #define GLEXT_GL_SRGB8_ALPHA8                     0
    #endif

#else

    #include "glLoader.hpp"

    // ODFAEG requires at a bare minimum OpenGL 1.1 capability
    // All functionality beyond that is optional
    // and has to be checked for prior to use

    // Core since 1.1
    #define GLEXT_GL_DEPTH_COMPONENT                  GL_DEPTH_COMPONENT
    #define GLEXT_GL_CLAMP                            GL_CLAMP

    // The following extensions are listed chronologically
    // Extension macro first, followed by tokens then
    // functions according to the corresponding specification

    // The following extensions are optional.

    // Core since 1.2 - SGIS_texture_edge_clamp
    #define GLEXT_texture_edge_clamp                  sfogl_ext_SGIS_texture_edge_clamp
    #define GLEXT_GL_CLAMP_TO_EDGE                    GL_CLAMP_TO_EDGE_SGIS

    // Core since 1.2 - EXT_texture_edge_clamp
    #define GLEXT_EXT_texture_edge_clamp              sfogl_ext_EXT_texture_edge_clamp

    // Core since 1.2 - EXT_blend_minmax
    #define GLEXT_blend_minmax                        sfogl_ext_EXT_blend_minmax
    #define GLEXT_glBlendEquation                     glBlendEquationEXT
    #define GLEXT_GL_FUNC_ADD                         GL_FUNC_ADD_EXT

    // Core since 1.2 - EXT_blend_subtract
    #define GLEXT_blend_subtract                      sfogl_ext_EXT_blend_subtract
    #define GLEXT_GL_FUNC_SUBTRACT                    GL_FUNC_SUBTRACT_EXT
    #define GLEXT_GL_FUNC_REVERSE_SUBTRACT            GL_FUNC_REVERSE_SUBTRACT_EXT

    // Core since 1.3 - ARB_multitexture
    #define GLEXT_multitexture                        sfogl_ext_ARB_multitexture
    #define GLEXT_glClientActiveTexture               glClientActiveTextureARB
    #define GLEXT_glActiveTexture                     glActiveTextureARB
    #define GLEXT_GL_TEXTURE0                         GL_TEXTURE0_ARB

    // Core since 1.4 - EXT_blend_func_separate
    #define GLEXT_blend_func_separate                 sfogl_ext_EXT_blend_func_separate
    #define GLEXT_glBlendFuncSeparate                 glBlendFuncSeparateEXT

    // Core since 1.5 - ARB_vertex_buffer_object
    #define GLEXT_vertex_buffer_object                sfogl_ext_ARB_vertex_buffer_object
    #define GLEXT_GL_ARRAY_BUFFER                     GL_ARRAY_BUFFER_ARB
    #define GLEXT_GL_DYNAMIC_DRAW                     GL_DYNAMIC_DRAW_ARB
    #define GLEXT_GL_READ_ONLY                        GL_READ_ONLY_ARB
    #define GLEXT_GL_STATIC_DRAW                      GL_STATIC_DRAW_ARB
    #define GLEXT_GL_STREAM_DRAW                      GL_STREAM_DRAW_ARB
    #define GLEXT_GL_WRITE_ONLY                       GL_WRITE_ONLY_ARB
    #define GLEXT_glBindBuffer                        glBindBufferARB
    #define GLEXT_glBufferData                        glBufferDataARB
    #define GLEXT_glBufferSubData                     glBufferSubDataARB
    #define GLEXT_glDeleteBuffers                     glDeleteBuffersARB
    #define GLEXT_glGenBuffers                        glGenBuffersARB
    #define GLEXT_glMapBuffer                         glMapBufferARB
    #define GLEXT_glUnmapBuffer                       glUnmapBufferARB

    // Core since 2.0 - ARB_shading_language_100
    #define GLEXT_shading_language_100                sfogl_ext_ARB_shading_language_100

    // Core since 2.0 - ARB_shader_objects
    #define GLEXT_shader_objects                      sfogl_ext_ARB_shader_objects
    #define GLEXT_glDeleteObject                      glDeleteObjectARB
    #define GLEXT_glGetHandle                         glGetHandleARB
    #define GLEXT_glCreateShaderObject                glCreateShaderObjectARB
    #define GLEXT_glShaderSource                      glShaderSourceARB
    #define GLEXT_glCompileShader                     glCompileShaderARB
    #define GLEXT_glCreateProgramObject               glCreateProgramObjectARB
    #define GLEXT_glAttachObject                      glAttachObjectARB
    #define GLEXT_glLinkProgram                       glLinkProgramARB
    #define GLEXT_glUseProgramObject                  glUseProgramObjectARB
    #define GLEXT_glUniform1f                         glUniform1fARB
    #define GLEXT_glUniform2f                         glUniform2fARB
    #define GLEXT_glUniform3f                         glUniform3fARB
    #define GLEXT_glUniform4f                         glUniform4fARB
    #define GLEXT_glUniform1i                         glUniform1iARB
    #define GLEXT_glUniform2i                         glUniform2iARB
    #define GLEXT_glUniform3i                         glUniform3iARB
    #define GLEXT_glUniform4i                         glUniform4iARB
    #define GLEXT_glUniform1fv                        glUniform1fvARB
    #define GLEXT_glUniform2fv                        glUniform2fvARB
    #define GLEXT_glUniform2iv                        glUniform2ivARB
    #define GLEXT_glUniform3fv                        glUniform3fvARB
    #define GLEXT_glUniform4fv                        glUniform4fvARB
    #define GLEXT_glUniformMatrix3fv                  glUniformMatrix3fvARB
    #define GLEXT_glUniformMatrix4fv                  glUniformMatrix4fvARB
    #define GLEXT_glGetObjectParameteriv              glGetObjectParameterivARB
    #define GLEXT_glGetInfoLog                        glGetInfoLogARB
    #define GLEXT_glGetUniformLocation                glGetUniformLocationARB
    #define GLEXT_GL_PROGRAM_OBJECT                   GL_PROGRAM_OBJECT_ARB
    #define GLEXT_GL_OBJECT_COMPILE_STATUS            GL_OBJECT_COMPILE_STATUS_ARB
    #define GLEXT_GL_OBJECT_LINK_STATUS               GL_OBJECT_LINK_STATUS_ARB
    #define GLEXT_GLhandle                            GLhandleARB

    // Core since 2.0 - ARB_vertex_shader
    #define GLEXT_vertex_shader                       sfogl_ext_ARB_vertex_shader
    #define GLEXT_GL_VERTEX_SHADER                    GL_VERTEX_SHADER_ARB
    #define GLEXT_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB

    // Core since 2.0 - ARB_fragment_shader
    #define GLEXT_fragment_shader                     sfogl_ext_ARB_fragment_shader
    #define GLEXT_GL_FRAGMENT_SHADER                  GL_FRAGMENT_SHADER_ARB

    // Core since 2.0 - ARB_texture_non_power_of_two
    #define GLEXT_texture_non_power_of_two            sfogl_ext_ARB_texture_non_power_of_two

    // Core since 2.0 - EXT_blend_equation_separate
    #define GLEXT_blend_equation_separate             sfogl_ext_EXT_blend_equation_separate
    #define GLEXT_glBlendEquationSeparate             glBlendEquationSeparateEXT

    // Core since 2.1 - EXT_texture_sRGB
    #define GLEXT_texture_sRGB                        sfogl_ext_EXT_texture_sRGB
    #define GLEXT_GL_SRGB8_ALPHA8                     GL_SRGB8_ALPHA8_EXT

    // Core since 3.0 - EXT_framebuffer_object
    #define GLEXT_framebuffer_object                  sfogl_ext_EXT_framebuffer_object
    #define GLEXT_glBindRenderbuffer                  glBindRenderbufferEXT
    #define GLEXT_glDeleteRenderbuffers               glDeleteRenderbuffersEXT
    #define GLEXT_glGenRenderbuffers                  glGenRenderbuffersEXT
    #define GLEXT_glRenderbufferStorage               glRenderbufferStorageEXT
    #define GLEXT_glBindFramebuffer                   glBindFramebufferEXT
    #define GLEXT_glDeleteFramebuffers                glDeleteFramebuffersEXT
    #define GLEXT_glGenFramebuffers                   glGenFramebuffersEXT
    #define GLEXT_glCheckFramebufferStatus            glCheckFramebufferStatusEXT
    #define GLEXT_glFramebufferTexture2D              glFramebufferTexture2DEXT
    #define GLEXT_glFramebufferRenderbuffer           glFramebufferRenderbufferEXT
    #define GLEXT_glGenerateMipmap                    glGenerateMipmapEXT
    #define GLEXT_GL_FRAMEBUFFER                      GL_FRAMEBUFFER_EXT
    #define GLEXT_GL_RENDERBUFFER                     GL_RENDERBUFFER_EXT
    #define GLEXT_GL_COLOR_ATTACHMENT0                GL_COLOR_ATTACHMENT0_EXT
    #define GLEXT_GL_DEPTH_ATTACHMENT                 GL_DEPTH_ATTACHMENT_EXT
    #define GLEXT_GL_FRAMEBUFFER_COMPLETE             GL_FRAMEBUFFER_COMPLETE_EXT
    #define GLEXT_GL_FRAMEBUFFER_BINDING              GL_FRAMEBUFFER_BINDING_EXT
    #define GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION    GL_INVALID_FRAMEBUFFER_OPERATION_EXT
    #define GLEXT_GL_STENCIL_ATTACHMENT               GL_STENCIL_ATTACHMENT_EXT

    // Core since 3.0 - EXT_packed_depth_stencil
    #define GLEXT_packed_depth_stencil                sfogl_ext_EXT_packed_depth_stencil
    #define GLEXT_GL_DEPTH24_STENCIL8                 GL_DEPTH24_STENCIL8_EXT

    // Core since 3.0 - EXT_framebuffer_blit
    #define GLEXT_framebuffer_blit                    sfogl_ext_EXT_framebuffer_blit
    #define GLEXT_glBlitFramebuffer                   glBlitFramebufferEXT
    #define GLEXT_GL_READ_FRAMEBUFFER                 GL_READ_FRAMEBUFFER_EXT
    #define GLEXT_GL_DRAW_FRAMEBUFFER                 GL_DRAW_FRAMEBUFFER_EXT
    #define GLEXT_GL_DRAW_FRAMEBUFFER_BINDING         GL_DRAW_FRAMEBUFFER_BINDING_EXT
    #define GLEXT_GL_READ_FRAMEBUFFER_BINDING         GL_READ_FRAMEBUFFER_BINDING_EXT

    // Core since 3.0 - EXT_framebuffer_multisample
    #define GLEXT_framebuffer_multisample             sfogl_ext_EXT_framebuffer_multisample
    #define GLEXT_glRenderbufferStorageMultisample    glRenderbufferStorageMultisampleEXT
    #define GLEXT_GL_MAX_SAMPLES                      GL_MAX_SAMPLES_EXT

    // Core since 3.1 - ARB_copy_buffer
    #define GLEXT_copy_buffer                         sfogl_ext_ARB_copy_buffer
    #define GLEXT_GL_COPY_READ_BUFFER                 GL_COPY_READ_BUFFER
    #define GLEXT_GL_COPY_WRITE_BUFFER                GL_COPY_WRITE_BUFFER
    #define GLEXT_glCopyBufferSubData                 glCopyBufferSubData

    // Core since 3.2 - ARB_geometry_shader4
    #define GLEXT_geometry_shader4                    sfogl_ext_ARB_geometry_shader4
    #define GLEXT_GL_GEOMETRY_SHADER                  GL_GEOMETRY_SHADER_ARB

#endif
namespace odfaeg
{
namespace graphic {
namespace priv
{

////////////////////////////////////////////////////////////
/// \brief Make sure that extensions are initialized
///
////////////////////////////////////////////////////////////
void ensureExtensionsInit();

} // namespace priv
}
}
#endif
