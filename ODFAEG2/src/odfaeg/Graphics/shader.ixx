module;
#include <vulkan/vulkan.hpp>
#include <string>
#include <shaderc/shaderc.hpp>
export module odfaeg.graphic.shader;
import odfaeg.core.inputStream;
import odfaeg.graphic.device;
namespace odfaeg{
	namespace graphic {
        export class  Shader {
        public:
            enum ShaderType {
                VERTEX_SHADER, COMPUTE_SHADER
            };
            ////////////////////////////////////////////////////////////
            /// \brief Special type/value that can be passed to setParameter,
            ///        and that represents the texture of the object being drawn
            ///
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            /// \brief Default constructor
            ///
            /// This constructor creates an invalid shader.
            ///
            ////////////////////////////////////////////////////////////
            Shader(Device& vkDevice);
            ////////////////////////////////////////////////////////////
            /// \brief Destructor
            ///
            ////////////////////////////////////////////////////////////
            ~Shader();
            ////////////////////////////////////////////////////////////
            /// \brief Load both the vertex and fragment shaders from files
            ///
            /// This function loads both the vertex and the fragment
            /// shaders. If one of them fails to load, the shader is left
            /// empty (the valid shader is unloaded).
            /// The sources must be text files containing valid shaders
            /// in GLSL language. GLSL is a C-like language dedicated to
            /// OpenGL shaders; you'll probably need to read a good documentation
            /// for it before writing your own shaders.
            ///
            /// \param vertexShaderFilename   Path of the vertex shader file to load
            /// \param fragmentShaderFilename Path of the fragment shader file to load
            ///
            /// \return True if loading succeeded, false if it failed
            ///
            /// \see loadFromMemory, loadFromStream
            ///
            ////////////////////////////////////////////////////////////
            bool loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename);
            ////////////////////////////////////////////////////////////
            /// \brief Load either the vertex or fragment shader from a source code in memory
            ///
            /// This function loads a single shader, either vertex or
            /// fragment, identified by the second argument.
            /// The source code must be a valid shader in GLSL language.
            /// GLSL is a C-like language dedicated to OpenGL shaders;
            /// you'll probably need to read a good documentation for
            /// it before writing your own shaders.
            ///
            /// \param shader String containing the source code of the shader
            /// \param type   Type of shader (vertex or fragment)
            ///
            /// \return True if loading succeeded, false if it failed
            ///
            /// \see loadFromFile, loadFromStream
            ///
            ////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////
            bool loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader);
            bool loadFromFile(const std::string& shaderFileName, ShaderType shaderType = COMPUTE_SHADER);
            ////////////////////////////////////////////////////////////
            /// \brief Load either the vertex or fragment shader from a custom stream
            ///
            /// This function loads a single shader, either vertex or
            /// fragment, identified by the second argument.
            /// The source code must be a valid shader in GLSL language.
            /// GLSL is a C-like language dedicated to OpenGL shaders;
            /// you'll probably need to read a good documentation for it
            /// before writing your own shaders.
            ///
            /// \param stream Source stream to read from
            /// \param type   Type of shader (vertex or fragment)
            ///
            /// \return True if loading succeeded, false if it failed
            ///
            /// \see loadFromFile, loadFromMemory
            ///
            bool loadFromStream(core::InputStream& vertexShaderStream, core::InputStream& fragmentShaderStream);
            ////////////////////////////////////////////////////////////
            /// \brief Bind a vertex attribute of the shader.
            /// The vertex attribute value's location is defined in the C code with the function glVertexAttributePointer.
            /// \a name is the name of the vertex attribute to bind in the shader.
            /// The corresponding attribute in the shader must be a float
            /// (float GLSL type).
            /// \a location is the location of the vertex attribute.
            /// this value must always be less than GL_MAX_VERTEX_ATTRIBS.
            /// The maximum vertex attrib is limited by your graphics hardware.
            /// Example:
            /// \code
            /// attribute vec3 vertex_position; // this is the variable in the shader
            /// \endcode
            /// \code
            /// shader.bindAtribute(0, "vertex_position");
            /// \endcode
            ///
            /// \param name Name of the vertex attribute in the shader
            /// \param location  Location of the attribute in the vertex.
            ///
            bool loadFromFile(const std::string& vertexFileName, const std::string& fragmentFileName, const std::string& geometryFileName);
            bool loadMeshFromFile(const std::string& meshShaderFileName, const std::string& fragmentShaderFileName, const std::string& taskShaderFileName="");
            bool loadMeshFromFileSpv(const std::string& meshShaderFileName, const std::string& fragmentShaderFileName, const std::string& taskShaderFileName="");
            bool loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader);
            bool loadRaytracingFromMemory(const std::string& raygenShader, const std::string& raymissShader, const std::string& rayhitShader);
            bool loadFromStream(core::InputStream& vertexShaderStream, core::InputStream& fragmentShaderStream, core::InputStream& geometryShaderStream);
            bool loadFromMemory(const std::string& shaderCode, ShaderType shaderType);
            template <typename T>
            void setParameter(VkCommandBuffer cmd, VkShaderStageFlags shaderStages, VkPipelineLayout pipelineLayout, unsigned int offset, T value) {
                vkCmdPushConstants(cmd, pipelineLayout, shaderStages, offset, sizeof(T), &value);
            }
            void createShaderModules();
            void createComputeShaderModule();
            void createMeshShaderModules();
            void createRaytracingShaderModules();
            void cleanupShaderModules();
            void cleanupMeshShaderModules();
            void cleanupComputeShaderModule();
            void cleanupRaytracingShaderModules();
            VkShaderModule getVertexShaderModule();
            VkShaderModule getFragmentShaderModule();
            VkShaderModule getGeometryShaderModule();
            VkShaderModule getComputeShaderModule();
            VkShaderModule getRaygenShaderModule();
            VkShaderModule getRaymissShaderModule();
            VkShaderModule getRayhitShaderModule();
            VkShaderModule getMeshShaderModule();
            VkShaderModule getTaskShaderModule();
            unsigned int getId() const;
            static unsigned int getNbShaders();           
            
            bool operator== (const Shader& shader) const;
            /**
            * \fn bool operator!= (const Material& material)
            * \brief test of two material are different.
            * \param material : the other material.
            * \return if the two material are different.
            */
            bool operator!= (Shader& shader);
        private:
            std::vector<VkPushConstantRange> pushConstants;

            ////////////////////////////////////////////////////////////
            /// \brief Compile the shader(s) and create the program
            ///
            /// If one of the arguments is NULL, the corresponding shader
            /// is not created.
            ///
            /// \param vertexShaderCode   Source code of the vertex shader
            /// \param fragmentShaderCode Source code of the fragment shader
            ///
            /// \return True on success, false if any error happened
            ///
            ////////////////////////////////////////////////////////////
            bool compile(const char* vertexShaderCode, const char* fragmentShaderCode, const char* geometryShaderCode = nullptr);
            bool compile(const char* shaderCode, ShaderType shadeType);
            bool compileRaytracing(const char* raygenShaderCode, const char* raymissShaderCode, const char* rayhitShaderCode = nullptr);
            bool compileMesh(const char* meshShaderCode, const char* fragmentShaderCode, const char* taskShaderCode = nullptr);
            bool isCompiled;
            std::string vertexShaderCode, fragmentShaderCode, geometryShaderCode, meshShaderCode, taskShaderCode, raygenShaderCode, raymissShaderCode, rayhitShaderCode;
            VkShaderModule vertexShaderModule, fragmentShaderModule, geometryShaderModule, computeShaderModule,
                meshShaderModule, taskShaderModule, raygenShaderModule, raymissShaderModule, rayhitShaderModule;
            std::vector<uint32_t> spvMeshShaderCode;
            std::vector<uint32_t> spvTaskShaderCode;            
            std::vector<uint32_t> spvVertexShaderCode;
            std::vector<uint32_t> spvGeometryShaderCode;
            std::vector<uint32_t> spvFragmentShaderCode;
            std::vector<uint32_t> spvComputeShaderCode;
            std::vector<uint32_t> spvRaygenShaderCode;
            std::vector<uint32_t> spvRaymissShaderCode;
            std::vector<uint32_t> spvRayhitShaderCode;
            Device& device;
            inline static unsigned int nbShaders = 0;
            unsigned int id;            
        };
	}
}