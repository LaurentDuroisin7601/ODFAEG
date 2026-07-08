module;
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <odfaeg/config.hpp>
//import odfaeg.graphic.shader;
module odfaeg.graphic.shader;
import odfaeg.core.inputStream;
namespace odfaeg {
	namespace graphic {
        namespace {
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
                bool getStreamContents(core::InputStream& stream, std::vector<char>& buffer)
                {

                    bool success = true;
                    std::int64_t size = stream.getSize();
                    if (size > 0)
                    {
                        buffer.resize(static_cast<std::size_t>(size));
                        stream.seek(0);
                        std::int64_t read = stream.read(&buffer[0], size);
                        success = (read == size);
                    }
                    buffer.push_back('\0');
                    return success;
                }
            }      
            Shader::Shader(Device& vkDevice) : device(vkDevice), fragmentShaderModule(nullptr), geometryShaderModule(nullptr), computeShaderModule(nullptr), taskShaderModule(nullptr), meshShaderModule(nullptr) {
                id = nbShaders;
                isCompiled = false;
                nbShaders++;
                geometryShaderCode = "";
            }
            bool Shader::operator==(const Shader& shader) const {
                return (vertexShaderCode == shader.vertexShaderCode) && (fragmentShaderCode == shader.fragmentShaderCode);
            }                    
            bool Shader::operator!= (Shader& shader) {
                return !(*this == shader);
            }
            unsigned int Shader::getId() const {
                return id;
            }
            unsigned int Shader::getNbShaders() {
                return nbShaders;
            }
            ////////////////////////////////////////////////////////////
            bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename)
            {
                // Read the vertex shader file
                std::vector<char> vertexShader;
                if (!getFileContents(vertexShaderFilename, vertexShader))
                {
                    std::cerr << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
                    return false;
                }

                // Read the fragment shader file
                std::vector<char> fragmentShader;
                if (!getFileContents(fragmentShaderFilename, fragmentShader))
                {
                    std::cerr << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                    return false;
                }
                vertexShaderCode = std::string(&vertexShader[0]);
                fragmentShaderCode = std::string(&fragmentShader[0]);
                // Compile the shader program
                return compile(&vertexShader[0], &fragmentShader[0]);
            }
            bool Shader::loadMeshFromFileSpv(const std::string& meshShaderFileName, const std::string& fragmentShaderFileName, const std::string& taskShaderFileName) {
                std::vector<char> meshShader;
                if (!getFileContents(meshShaderFileName, meshShader))
                {
                    std::cerr << "Failed to open mesh shader file \"" << meshShaderFileName << "\"" << std::endl;
                    return false;
                }

                // Read the fragment shader file
                std::vector<char> fragmentShader;
                if (!getFileContents(fragmentShaderFileName, fragmentShader))
                {
                    std::cerr << "Failed to open fragment shader file \"" << fragmentShaderFileName << "\"" << std::endl;
                    return false;
                }
                std::vector<char> taskShader;
                if (!taskShaderFileName.empty() && !getFileContents(taskShaderFileName, taskShader)) {
                    std::cerr << "Failed to open task shader file \"" << taskShaderFileName << "\"" << std::endl;
                    return false;
                }
                meshShaderCode = std::string(&meshShader[0]);
                fragmentShaderCode = std::string(&fragmentShader[0]);
                if (taskShader.size() > 0) {
                    taskShaderCode = std::string(&taskShader[0]);
                }                
                return true;
            }
            ////////////////////////////////////////////////////////////
            bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader)
            {
                // Compile the shader program
                vertexShaderCode = vertexShader;
                fragmentShaderCode = fragmentShader;
                return compile(vertexShader.c_str(), fragmentShader.c_str());
            }
            bool Shader::loadRaytracingFromMemory(const std::string& raygenShader, const std::string& raymissShader, const std::string& rayhitShader) {
                return compileRaytracing(raygenShader.c_str(), raymissShader.c_str(), rayhitShader.c_str());
            }
            ////////////////////////////////////////////////////////////
            bool Shader::loadFromStream(core::InputStream& vertexShaderStream, core::InputStream& fragmentShaderStream)
            {
                // Read the vertex shader code from the stream
                std::vector<char> vertexShader;
                if (!getStreamContents(vertexShaderStream, vertexShader))
                {
                    std::cerr << "Failed to read vertex shader from stream" << std::endl;
                    return false;
                }

                // Read the fragment shader code from the stream
                std::vector<char> fragmentShader;
                if (!getStreamContents(fragmentShaderStream, fragmentShader))
                {
                    std::cerr << "Failed to read fragment shader from stream" << std::endl;
                    return false;
                }
                vertexShaderCode = std::string(&vertexShader[0]);
                fragmentShaderCode = std::string(&fragmentShader[0]);
                // Compile the shader program
                return compile(&vertexShader[0], &fragmentShader[0]);
            }
            bool Shader::loadFromFile(const std::string& vertexShaderFilename, const std::string& fragmentShaderFilename, const std::string& geometryShaderFilename) {
                std::vector<char> vertexShader;
                if (!getFileContents(vertexShaderFilename, vertexShader))
                {
                    std::cerr << "Failed to open vertex shader file \"" << vertexShaderFilename << "\"" << std::endl;
                    return false;
                }

                // Read the fragment shader file
                std::vector<char> fragmentShader;
                if (!getFileContents(fragmentShaderFilename, fragmentShader))
                {
                    std::cerr << "Failed to open fragment shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                    return false;
                }
                std::vector<char> geometryShader;
                if (!getFileContents(geometryShaderFilename, geometryShader)) {
                    std::cerr << "Failed to open geometry shader file \"" << fragmentShaderFilename << "\"" << std::endl;
                    return false;
                }
                vertexShaderCode = std::string(&vertexShader[0]);
                fragmentShaderCode = std::string(&fragmentShader[0]);
                geometryShaderCode = std::string(&geometryShader[0]);
                return compile(&vertexShader[0], &fragmentShader[0], &geometryShader[0]);
            } 
            bool Shader::loadFromMemory(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader) {
                vertexShaderCode = vertexShader;
                fragmentShaderCode = fragmentShader;
                geometryShaderCode = geometryShader;
                return compile(vertexShader.c_str(), fragmentShader.c_str(), geometryShader.c_str());
            }
            bool Shader::loadFromStream(core::InputStream& vertexShaderStream, core::InputStream& fragmentShaderStream, core::InputStream& geometryShaderStream) {
                // Read the vertex shader code from the stream
                std::vector<char> vertexShader;
                if (!getStreamContents(vertexShaderStream, vertexShader))
                {
                    std::cerr << "Failed to read vertex shader from stream" << std::endl;
                    return false;
                }

                // Read the fragment shader code from the stream
                std::vector<char> fragmentShader;
                if (!getStreamContents(fragmentShaderStream, fragmentShader))
                {
                    std::cerr << "Failed to read fragment shader from stream" << std::endl;
                    return false;
                }

                std::vector<char> geometryShader;
                if (!getStreamContents(geometryShaderStream, geometryShader))
                {
                    std::cerr << "Failed to read fragment shader from stream" << std::endl;
                    return false;
                }
                vertexShaderCode = std::string(&vertexShader[0]);
                fragmentShaderCode = std::string(&fragmentShader[0]);
                geometryShaderCode = std::string(&geometryShader[0]);
                // Compile the shader program
                return compile(&vertexShader[0], &fragmentShader[0], &geometryShader[0]);
            }
            bool Shader::loadFromMemory(const std::string& shaderCode, ShaderType shaderType) {
                return compile(shaderCode.c_str(), shaderType);
            }
            bool Shader::loadFromFile(const std::string& shaderFileName, ShaderType shaderType) {
                std::vector<char> shader;
                if (!getFileContents(shaderFileName, shader))
                {
                    std::cerr << "Failed to open compute shader file \"" << shaderFileName << "\"" << std::endl;
                    return false;
                }
                return compile(&shader[0], shaderType);
            }
            bool Shader::compile(const char* shaderCode, ShaderType shaderType) {
                shaderc::Compiler compiler;
                shaderc::CompileOptions options;
                options.SetOptimizationLevel(shaderc_optimization_level_size);
                shaderc::SpvCompilationResult module;
                switch (shaderType) {
                    case VERTEX_SHADER : {
                        module = compiler.CompileGlslToSpv(shaderCode, shaderc_glsl_vertex_shader, "shader_src", options); 
                        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                            std::cerr << "Failed to compile vertex shader :  " << module.GetErrorMessage();
                            return false;
                        }
                        else {
                            spvVertexShaderCode = { module.cbegin(), module.cend() };                    
                        }
                        break;
                    }
                    case COMPUTE_SHADER : {
                         module = compiler.CompileGlslToSpv(shaderCode, shaderc_glsl_compute_shader, "shader_src", options); 
                         if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                            std::cerr << "Failed to compile compute shader :  " << module.GetErrorMessage();
                            return false;
                            }
                            else {
                                spvComputeShaderCode = { module.cbegin(), module.cend() };                    
                            }
                            break;
                    }
                };                
                isCompiled = true;
                return true;
            }
            ////////////////////////////////////////////////////////////
            bool Shader::compile(const char* vertexShaderCode, const char* fragmentShaderCode, const char* geometryShaderCode) {
                shaderc::Compiler compiler;
                shaderc::CompileOptions options;
                #ifdef ODFAEG_DEBUG
                options.SetGenerateDebugInfo();
                options.SetOptimizationLevel(shaderc_optimization_level_zero);
                #else
                options.SetOptimizationLevel(shaderc_optimization_level_size);
                #endif
                shaderc::SpvCompilationResult module =
                    compiler.CompileGlslToSpv(vertexShaderCode, shaderc_glsl_vertex_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile vertex shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvVertexShaderCode = { module.cbegin(), module.cend() };
                }
                module = compiler.CompileGlslToSpv(fragmentShaderCode, shaderc_glsl_fragment_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile fragment shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvFragmentShaderCode = { module.cbegin(), module.cend() };
                }
                if (geometryShaderCode != nullptr) {
                    module = compiler.CompileGlslToSpv(geometryShaderCode, shaderc_glsl_geometry_shader, "shader_src", options);
                    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                        std::cerr << "Failed to compile vertex shader :  " << module.GetErrorMessage();
                        return false;
                    }
                    else {
                        spvGeometryShaderCode = { module.cbegin(), module.cend() };
                    }
                }
                isCompiled = true;
                //updateIds();
                return true;
            }
            bool Shader::compileMesh(const char* meshShaderCode, const char* fragmentShaderCode, const char* taskShaderCode) {
                shaderc::Compiler compiler;
                shaderc::CompileOptions options;
                options.SetOptimizationLevel(shaderc_optimization_level_size);
                shaderc::SpvCompilationResult module =
                    compiler.CompileGlslToSpv(meshShaderCode, shaderc_glsl_mesh_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile mesh shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvMeshShaderCode = { module.cbegin(), module.cend() };
                }
                module = compiler.CompileGlslToSpv(fragmentShaderCode, shaderc_glsl_fragment_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile fragment shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvFragmentShaderCode = { module.cbegin(), module.cend() };
                }
                if (taskShaderCode != nullptr) {
                    module = compiler.CompileGlslToSpv(taskShaderCode, shaderc_glsl_task_shader, "shader_src", options);
                    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                        std::cerr << "Failed to compile task shader :  " << module.GetErrorMessage();
                        return false;
                    }
                    else {
                        spvTaskShaderCode = { module.cbegin(), module.cend() };
                    }
                }
                isCompiled = true;
                //updateIds();
                return true;
            }
            bool Shader::compileRaytracing(const char* raygenShaderCode, const char* raymissShaderCode, const char* rayhitShaderCode) {
                shaderc::Compiler compiler;
                shaderc::CompileOptions options;
                options.SetOptimizationLevel(shaderc_optimization_level_size);
                shaderc::SpvCompilationResult module =
                    compiler.CompileGlslToSpv(vertexShaderCode, shaderc_glsl_raygen_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile raygen shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvRaygenShaderCode = { module.cbegin(), module.cend() };
                }
                module = compiler.CompileGlslToSpv(fragmentShaderCode, shaderc_glsl_miss_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile raymiss shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvRaymissShaderCode = { module.cbegin(), module.cend() };
                }
                module = compiler.CompileGlslToSpv(geometryShaderCode, shaderc_glsl_closesthit_shader, "shader_src", options);
                if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
                    std::cerr << "Failed to compile rayhit shader :  " << module.GetErrorMessage();
                    return false;
                }
                else {
                    spvRayhitShaderCode = { module.cbegin(), module.cend() };
                }
                isCompiled = true;
                //updateIds();
                return true;
            }
            void Shader::createShaderModules() {
                VkShaderModuleCreateInfo createVSInfo{};
                createVSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createVSInfo.codeSize = 4 * spvVertexShaderCode.size();
                createVSInfo.pCode = spvVertexShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createVSInfo, nullptr, &vertexShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create vertex shader module");
                }
                if (fragmentShaderCode != "") {
                    VkShaderModuleCreateInfo createFSInfo{};
                    createFSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    createFSInfo.codeSize = 4 * spvFragmentShaderCode.size();
                    createFSInfo.pCode = spvFragmentShaderCode.data();
                    if (vkCreateShaderModule(device.getDevice(), &createFSInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create fragment shader module");
                    }
                }
                if (geometryShaderCode != "") {
                    VkShaderModuleCreateInfo createGSInfo{};
                    createGSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    createGSInfo.codeSize = 4 * spvGeometryShaderCode.size();
                    createGSInfo.pCode = spvGeometryShaderCode.data();
                    if (vkCreateShaderModule(device.getDevice(), &createGSInfo, nullptr, &geometryShaderModule) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create geometry shader module");
                    }
                }
            }
            void Shader::createMeshShaderModules() {
                VkShaderModuleCreateInfo createMSInfo{};
                createMSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createMSInfo.codeSize = 4 * spvMeshShaderCode.size();
                createMSInfo.pCode = spvMeshShaderCode.data();
                std::cout<<"mesh shader code : "<<meshShaderCode<<std::endl;
                if (vkCreateShaderModule(device.getDevice(), &createMSInfo, nullptr, &meshShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create mesh shader module");
                }
                VkShaderModuleCreateInfo createFSInfo{};
                createFSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createFSInfo.codeSize = 4 * spvFragmentShaderCode.size();
                createFSInfo.pCode = spvFragmentShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createFSInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create fragment shader module");
                }
                //std::cout<<"task shader code : "<<taskShaderCode<<std::endl;
                if (taskShaderCode != "") {
                    VkShaderModuleCreateInfo createTSInfo{};
                    createTSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                    createTSInfo.codeSize = 4 * spvTaskShaderCode.size();
                    createTSInfo.pCode = spvTaskShaderCode.data();
                    if (vkCreateShaderModule(device.getDevice(), &createTSInfo, nullptr, &taskShaderModule) != VK_SUCCESS) {
                        throw std::runtime_error("Failed to create task shader module");
                    }
                }
            }
            void Shader::cleanupMeshShaderModules() {
                vkDestroyShaderModule(device.getDevice(), meshShaderModule, nullptr);
                vkDestroyShaderModule(device.getDevice(), fragmentShaderModule, nullptr);
                if (taskShaderModule != nullptr) {
                    vkDestroyShaderModule(device.getDevice(), taskShaderModule, nullptr);
                }
            }   
            VkShaderModule Shader::getMeshShaderModule() {
                return meshShaderModule;
            }
            VkShaderModule Shader::getTaskShaderModule() {
                return taskShaderModule;
            }
            void Shader::createComputeShaderModule() {
                VkShaderModuleCreateInfo createVSInfo{};
                createVSInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createVSInfo.codeSize = 4 * spvComputeShaderCode.size();
                createVSInfo.pCode = spvComputeShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createVSInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create compute shader module");
                }
            }
            void Shader::createRaytracingShaderModules() {
                VkShaderModuleCreateInfo createRGInfo{};
                createRGInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createRGInfo.codeSize = 4 * spvRaygenShaderCode.size();
                createRGInfo.pCode = spvRaygenShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createRGInfo, nullptr, &raygenShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create raygen shader module");
                }
                VkShaderModuleCreateInfo createRMInfo{};
                createRMInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createRMInfo.codeSize = 4 * spvFragmentShaderCode.size();
                createRMInfo.pCode = spvFragmentShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createRMInfo, nullptr, &raymissShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create raymiss shader module");
                }
                VkShaderModuleCreateInfo createRHInfo{};
                createRHInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createRHInfo.codeSize = 4 * spvGeometryShaderCode.size();
                createRHInfo.pCode = spvGeometryShaderCode.data();
                if (vkCreateShaderModule(device.getDevice(), &createRHInfo, nullptr, &rayhitShaderModule) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create rayhit shader module");
                }
            }
            void Shader::cleanupShaderModules() {
                vkDestroyShaderModule(device.getDevice(), vertexShaderModule, nullptr);
                if (fragmentShaderModule != nullptr) {
                    vkDestroyShaderModule(device.getDevice(), fragmentShaderModule, nullptr);
                }
                if (geometryShaderModule != nullptr) {
                    vkDestroyShaderModule(device.getDevice(), geometryShaderModule, nullptr);
                }
            }
            void Shader::cleanupComputeShaderModule() {
                if (computeShaderModule != nullptr) {
                    vkDestroyShaderModule(device.getDevice(), computeShaderModule, nullptr);
                }
            }
            void Shader::cleanupRaytracingShaderModules() {
                vkDestroyShaderModule(device.getDevice(), raygenShaderModule, nullptr);
                vkDestroyShaderModule(device.getDevice(), raymissShaderModule, nullptr);
                vkDestroyShaderModule(device.getDevice(), rayhitShaderModule, nullptr);
            }
            VkShaderModule Shader::getVertexShaderModule() {
                return vertexShaderModule;
            }
            VkShaderModule Shader::getFragmentShaderModule() {
                return fragmentShaderModule;
            }
            VkShaderModule Shader::getGeometryShaderModule() {
                return geometryShaderModule;
            }
            VkShaderModule Shader::getRaygenShaderModule() {
                return raygenShaderModule;
            }
            VkShaderModule Shader::getRaymissShaderModule() {
                return raymissShaderModule;
            }
            VkShaderModule Shader::getRayhitShaderModule() {
                return rayhitShaderModule;
            }
            VkShaderModule Shader::getComputeShaderModule() {
                return computeShaderModule;
            }
            Shader::~Shader() {
            }
	}
}