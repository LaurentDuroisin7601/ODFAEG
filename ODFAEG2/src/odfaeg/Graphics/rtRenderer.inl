namespace odfaeg {
    namespace graphic {
        RTRenderer::RTRenderer(RenderTarget& parentRenderer, Texture& environmentMap, RenderTexture& frameBuffer,
                    RenderTexture& cmsShadowMaps, RenderTexture& plShadowMaps, unsigned int layer, std::string typesToRenderExpression, int windowId, bool usethread) :
        parentRenderer(parentRenderer),
        environementMap(environmentMap),
        frameBuffer(frameBuffer),
        csmShadowMaps(cmsShadowMaps),
        plShadowMap(plShadowMaps),
        rtShader(GPUContext::instance().getDevice()),
        commandPool(GPUContext::instance().getDevice()),
        transformMatrixBuffer(GPUContext::instance().getDevice()),
        materialBuffer(GPUContext::instance().getDevice()),
        geometryOffsetBuffer(GPUContext::instance().getDevice()),
        instancesBuffer(GPUContext::instance().getDevice()),
        transformMatrixStaggingBuffer(GPUContext::instance().getDevice()),
        materialStaggingBuffer(GPUContext::instance().getDevice()),
        geometryOffsetStaggingBuffer(GPUContext::instance().getDevice()),
        instancesStaggingBuffer(GPUContext::instance().getDevice()),
        raygenShaderBT(GPUContext::instance().getDevice()),
        raymissShaderBT(GPUContext::instance().getDevice()),
        rayhitShaderBT(GPUContext::instance().getDevice()),
        dirLightStaggingBuffer(GPUContext::instance().getDevice()),
        pointLightStaggingBuffer(GPUContext::instance().getDevice());
        typesToRenderExpression(typesToRenderExpression)        
        {
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
            VkPhysicalDeviceProperties2 deviceProperties2{};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(GPUContext::instance().getDevice().getPhysicalDevice(), &deviceProperties2);

            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(GPUContext::instance().getDevice().getPhysicalDevice(), parentRenderer.getImageFormat(), &props);
            VkFormat storageFormat;
            if (!(props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)) {
                // Choisir un format compatible
                
                storageFormat = VK_FORMAT_R8G8B8A8_UNORM;
            } else {
                storageFormat = parentRenderer.getImageFormat();
            }
            //storageFormat = VK_FORMAT_R8G8B8A8_SRGB;
            math::Vector2u size = parentRenderer.getSize();
            //std::cout<<"size : "<<size<<std::endl;
            createCommandPool();
            commandPool.beginRecordCommandBuffer(0);
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                storageImage.emplace_back(GPUContext::instance().getDevice());
                storageImage.back().create(size.x(), size.y(), 1, VK_IMAGE_TYPE_2D, storageFormat, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                        1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                storageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, storageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
                Texture::transitionImageLayout(storageImage.back(), commandPool.getHandle(0), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            } 
            dirLightBuffer.emplace_back(GPUContext::instance().getDevice());
            pointLightBuffer.emplace_back(GPUContext::instance().getDevice());
            dirLightBuffer.back().create(sizeof(entity::DirectionnalLight), VK_BUFFER_USAGE_SHADER_STORAG_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            pointLightBuffer.back().create(sizeof(entity::DirectionnalLight), VK_BUFFER_USAGE_SHADER_STORAG_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            commandPool.endRecordCommandBuffer(0);
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(0);
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());
            UBOData uboDatas;
            uboDatas.projInverse = parentRenderer.getCamera().getProjMatrix().getMatrix().inverse().transpose(); 
            uboDatas.viewInverse = parentRenderer.getCamera().getViewMatrix().getMatrix().inverse().transpose();          
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                ubo.emplace_back(GPUContext::instance().getDevice());
                ubo.back().create(sizeof(UBOData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                ubo.back().update(&uboDatas, sizeof(UBOData));  
            }
            
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!rtShader.loadRaytracingFromFileSpv(shaderDir + "/raygenShader.rgen.spv", shaderDir + "/raymissShader.rmiss.spv", shaderDir+"/rayhitShader.rchit.spv")) {
                throw std::runtime_error("Could not load rt shader");
            }
            shaderGroupCount = 3;
            loadExtensionsFuncPtr();
            createDescriptorsAndPipelines();
            createShaderBindingTable();
            
            needToUpdateBLAS = needToUpdateTLAS = needToUpdateDescriptorSets = true; 
            /*updateBLAS();
            updateTLAS();*/
        } 
        void RTRenderer::addPointLight(entity::PointLight pointLight) {
            pointsLight.push_back(pointLight);
            needToUpdateBuffers = true;
        }
        void RTRenderer::addDirectionnalLight(entity::DirectionnalLight directionnalLight) {
            dirLights.push_back(directionnalLight);
            needToUpdateBuffers = true;
        }
        void RTRenderer::updateBuffers() {
            dirLightStaggingBuffer.create(sizeof(DirLight) * dirLights.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_BIT);
            dirLightStaggingBuffer.update(dirLights.data(), sizeof(DirLight) * dirLights.size());
            dirLightsBuffer.back().create(sizeof(DirLight) * dirLights.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_BIT);
            pointLightStaggingBuffer.create(sizeof(PointLight) * pointLights.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_BIT);
            pointLightStaggingBuffer.update(pointLights.data(), sizeof(PointLight) * pointLights.size());
            pointLightsBuffer.back().create(sizeof(PointLight) * pointLights.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_BIT);
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            Buffer::copyBuffer(dirLightStaggingBuffer, dirLightsBuffer.back(), commandPool.getHandle(parentRenderer.getCurrentFrame()));
            Buffer::copyBuffer(pointLightStaggingBuffer, pointLightsBuffer.back(), commandPool.getHandle(parentRenderer.getCurrentFrame()));
            commandPool.endRecordCommandBuffer(parentRenderer.getCurrentFrame());
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(parentRenderer.getCurrentFrame());
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("�chec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());
        }
        void RTRenderer::loadExtensionsFuncPtr() {
            VkDevice device = GPUContext::instance().getDevice().getDevice();
            // Get the ray tracing and accelertion structure related function pointers required by this sample            
            vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
            vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
            vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
            vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
            vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
            vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
            vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
            vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
           
        }
        void RTRenderer::createCommandPool() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
        }
        VkTransformMatrixKHR RTRenderer::toVulkanMatrix (math::Matrix4f matrix) {
            VkTransformMatrixKHR transformMatrix = {
                    matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
                    matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
                    matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3] };
            return transformMatrix;
        }
        void RTRenderer::updateBLAS() { 
            VkTransformMatrixKHR transformMatrix = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f
            };
            transformMatrixStaggingBuffer.create(sizeof(VkTransformMatrixKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);         
            transformMatrixStaggingBuffer.update(&transformMatrix, sizeof(VkTransformMatrixKHR));
            transformMatrixBuffer.create(sizeof(VkTransformMatrixKHR), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY, 0, true);
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            Buffer::copyBuffer(transformMatrixStaggingBuffer, transformMatrixBuffer, sizeof(VkTransformMatrixKHR), commandPool.getHandle(parentRenderer.getCurrentFrame()));
            std::vector<VkTransformMatrixKHR> transformMatrices;
            instancesGroupCount = singleInstancesCount = 0; 
            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

            vertexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(RenderTarget::VERTEX_BUFFER)[entity::Triangles].getVertexBuffer(0).getDeviceAddress();
            indexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(RenderTarget::VERTEX_BUFFER)[entity::Triangles].getIndexBuffer(0).getDeviceAddress();
            transformBufferDeviceAddress.deviceAddress = transformMatrixBuffer.getDeviceAddress();
            //std::cout<<"adresse : "<<vertexBufferDeviceAddress.deviceAddress<<","<<indexBufferDeviceAddress.deviceAddress<<","<<transformBufferDeviceAddress.deviceAddress<<std::endl;
            std::deque<Material*> materials = Material::getAllMaterials();
            std::vector<MaterialData> materialDatas;
            std::deque<Buffer> scratchBuffers;
            //std::cout<<"material : "<<materials.size()<<std::endl;
            for (unsigned int i = 0; i < materials.size(); i++) {
               
                MaterialData material;
                //std::cout<<"material : "<<materials[i]<<std::endl;
                material.diffuseTextureIndex = (materials[i]->getTexture(entity::SubMesh::DIFFUSE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::DIFFUSE)->getId() : 0;
                //std::cout<<"diffuse texture index : "<<material.diffuseTextureIndex<<std::endl;
                //system("PAUSE");
                material.specularTextureIndex = (materials[i]->getTexture(entity::SubMesh::SPECULAR) != nullptr) ? materials[i]->getTexture(entity::SubMesh::SPECULAR)->getId() : 0;
                material.normalTextureIndex = (materials[i]->getTexture(entity::SubMesh::NORMAL) != nullptr) ? materials[i]->getTexture(entity::SubMesh::NORMAL)->getId() : 0;
                material.metalnessTextureIndex = (materials[i]->getTexture(entity::SubMesh::METALNESS) != nullptr) ? materials[i]->getTexture(entity::SubMesh::METALNESS)->getId() : 0;
                material.roughnessTextureIndex = (materials[i]->getTexture(entity::SubMesh::ROUGHNESS) != nullptr) ? materials[i]->getTexture(entity::SubMesh::ROUGHNESS)->getId() : 0;
                material.aoTextureIndex = (materials[i]->getTexture(entity::SubMesh::AO) != nullptr) ? materials[i]->getTexture(entity::SubMesh::AO)->getId() : 0;
                material.emissiveTextureIndex = (materials[i]->getTexture(entity::SubMesh::EMISSIVE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::EMISSIVE)->getId() : 0;
                /*material.uvScale = /*(materials[i]->getTexture(Material::DIFFUSE) != nullptr) ? math::Vec2f(1.f / materials[i]->getTexture(Material::DIFFUSE)->getSize().x(), 1.f / materials[i]->getTexture(Material::DIFFUSE)->getSize().y()) :*/ /*math::Vec2f(1.f, 1.f);
                material.uvOffset = math::Vec2f(0.f, 0.f);*/
                material.materialType = materials[i]->getType();
                material.nbBuffers = (materials[i]->getTexture(entity::SubMesh::DIFFUSE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::DIFFUSE)->getNbBuffers() : 0;
                material.vertsInstanceSet  = 0;
                material.nbVertices = 0;
                material.nbIndexes = 0;
                material.materialId = materials[i]->getId();
                
                material.instanceGroupId = materials[i]->getInstanceGroupId();
                material.reflectable = (materials[i]->isReflectable()) ? 1 : 0;
                material.refractable = (materials[i]->isRefractable()) ? 1 : 0;
                /*std::cout<<"id : "<<material.materialId<<", reflectable : "<<material.reflectable<<"refractable : "<<material.refractable<<std::endl;
                system("PAUSE");*/
                /*if (material.instanceGroupId != -1)
                    system("PAUSE");**/
                materialDatas.push_back(material);

            }
            materialStaggingBuffer.create(sizeof(MaterialData)*materialDatas.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);         
            materialStaggingBuffer.update(materialDatas.data(), sizeof(MaterialData)*materialDatas.size());
            materialBuffer.create(sizeof(MaterialData)*materialDatas.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            
            Buffer::copyBuffer(materialStaggingBuffer, materialBuffer, sizeof(MaterialData)*materialDatas.size(), commandPool.getHandle(parentRenderer.getCurrentFrame()));
            std::vector<Mesh*>& gameObjects = RenderTarget::getGameObjects();
            std::vector<GeometryOffset> geometryOffsets;
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    entity::SubMesh sm = gameObjects[i]->getGameObject()->getSubMeshes()[j];
                    //Ensuite on parcours les matériaux à plusieurs instances.
                    if (gameObjects[i]->getMaterials()[0]->getInstanceGroupId() != -1) {
                        if (gameObjects[i]->getMaterials()[0]->materialSet == 0) {
                            gameObjects[i]->getMaterials()[0]->materialSet = 1;
                            VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
                            accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
                            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
                            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                            accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                            accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                            accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
                            accelerationStructureGeometry.geometry.triangles.maxVertex = sm.getVertexArray().getVertexCount();
                            accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(entity::Vertex);
                            accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
                            accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
                            accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
                            accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
                            accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;
                            
                            VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                            accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                            accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                            accelerationStructureBuildGeometryInfo.geometryCount = 1;
                            accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                            std::array<entity::VertexArray::LODLevel, 5> lods = sm.getVertexArray().getLODs();
                            const uint32_t numTriangles = lods[0].indexCount / 3;
                            VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                            accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                            vkGetAccelerationStructureBuildSizesKHR(
                            GPUContext::instance().getDevice().getDevice(),
                            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                            &accelerationStructureBuildGeometryInfo,
                            &numTriangles,
                            &accelerationStructureBuildSizesInfo);
                            bottomLevelASBuffers.emplace_back(GPUContext::instance().getDevice()); 
                            bottomLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY, 0, true);
                            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                            accelerationStructureCreateInfo.buffer = bottomLevelASBuffers.back().getHandle();
                            accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            VkAccelerationStructureKHR handle;
                            vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &handle);

                            // Create a small scratch buffer used during build of the bottom level acceleration structure
                            scratchBuffers.emplace_back(GPUContext::instance().getDevice());
                            scratchBuffers.back().create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);


                            VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                            accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                            accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                            accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                            accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
                            accelerationBuildGeometryInfo.geometryCount = 1;
                            accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                            accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffers.back().getDeviceAddress();

                            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                            accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                            accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset * sizeof(std::uint32_t);
                            accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset * sizeof(entity::Vertex);
                            accelerationStructureBuildRangeInfo.transformOffset = 0;
                            
                            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                            vkCmdBuildAccelerationStructuresKHR(
                            commandPool.getHandle(parentRenderer.getCurrentFrame()),
                            1,
                            &accelerationBuildGeometryInfo,
                            accelerationBuildStructureRangeInfos.data());
                            
                            GeometryOffset geometryOffset;
                            geometryOffset.vertexOffset = sm.vertexOffset;
                            geometryOffset.indexOffset = sm.indexOffset + lods[0].indexOffset;   
                            geometryOffset.materialOffset = sm.materialId;
                            unsigned instanceID = gameObjects[i]->getMaterials()[0]->getInstanceGroupId();
                            if (instanceID >= geometryOffsets.size())
                                geometryOffsets.resize(instanceID);          
                            geometryOffsets[instanceID] = geometryOffset;
                            instancesGroupCount++;
                            gameObjects[i]->getMaterials()[0]->materialSet++;
                                                 
                        } else {
                            gameObjects[i]->getMaterials()[0]->materialSet++;
                        }
                    }
                }
            }
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    entity::SubMesh sm = gameObjects[i]->getGameObject()->getSubMeshes()[j];
                    //On commence par les matériaux à une seule instance.
                    if (gameObjects[i]->getMaterials()[0]->getInstanceGroupId() == -1) {
                    
                        
                        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
                        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
                        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
                        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
                        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
                        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
                        accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
                        accelerationStructureGeometry.geometry.triangles.maxVertex = sm.getVertexArray().getVertexCount();
                        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(entity::Vertex);
                        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
                        accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
                        accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
                        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;
                        accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;
                        
                        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                        accelerationStructureBuildGeometryInfo.geometryCount = 1;
                        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                        std::array<entity::VertexArray::LODLevel, 5> lods = sm.getVertexArray().getLODs();
                        const uint32_t numTriangles = lods[0].indexCount / 3;
                        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                        vkGetAccelerationStructureBuildSizesKHR(
                        GPUContext::instance().getDevice().getDevice(),
                        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                        &accelerationStructureBuildGeometryInfo,
                        &numTriangles,
                        &accelerationStructureBuildSizesInfo);
                        bottomLevelASBuffers.emplace_back(GPUContext::instance().getDevice()); 
                        bottomLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY, 0, true);
                        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                        accelerationStructureCreateInfo.buffer = bottomLevelASBuffers.back().getHandle();
                        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        VkAccelerationStructureKHR handle;
                        vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &handle);

                        // Create a small scratch buffer used during build of the bottom level acceleration structure
                        scratchBuffers.emplace_back(GPUContext::instance().getDevice());
                        scratchBuffers.back().create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);
                        /*std::cout<<"device adr : "<<scratchBuffer.getDeviceAddress()<<std::endl;
                        system("PAUSE");*/

                        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                        accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
                        accelerationBuildGeometryInfo.geometryCount = 1;
                        accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffers.back().getDeviceAddress();

                        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                        accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                        accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset * sizeof(std::uint32_t);
                        accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset;
                        accelerationStructureBuildRangeInfo.transformOffset = 0;
                        
                        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                        vkCmdBuildAccelerationStructuresKHR(
                        commandPool.getHandle(parentRenderer.getCurrentFrame()),
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data());
                        
                        GeometryOffset geometryOffset;
                        geometryOffset.vertexOffset = sm.vertexOffset;
                        geometryOffset.indexOffset = sm.indexOffset + lods[0].indexOffset;
                        geometryOffset.materialOffset = sm.materialId;                        
                        geometryOffsets.push_back(geometryOffset);                        
                        singleInstancesCount++;                         
                    } 
                } 
            }            
            VkDeviceSize bufferSize = geometryOffsets.size() * sizeof(GeometryOffset);
            geometryOffsetStaggingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);    
            geometryOffsetStaggingBuffer.update(geometryOffsets.data(), bufferSize);         
            geometryOffsetBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            
            Buffer::copyBuffer(geometryOffsetStaggingBuffer, geometryOffsetBuffer, bufferSize, commandPool.getHandle(parentRenderer.getCurrentFrame()));
            commandPool.endRecordCommandBuffer(parentRenderer.getCurrentFrame());
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(parentRenderer.getCurrentFrame());
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());                 
        }
        void RTRenderer::updateTLAS() {
            std::vector<VkAccelerationStructureInstanceKHR> instances;
            unsigned int singleInstancesIndex = 0;
            //instances.resize(singleInstancesCount+instancesGroupCount+1);
            unsigned int totalInstancesCount = 0;
            std::vector<Mesh*> gameObjects = RenderTarget::getGameObjects();
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    unsigned int instanceID;
                    if (gameObjects[i]->getMaterials()[0]->getInstanceGroupId() == -1) {
                        instanceID = singleInstancesIndex + instancesGroupCount;
                        singleInstancesIndex++;
                    } else {
                        instanceID = gameObjects[i]->getMaterials()[0]->getInstanceGroupId();
                    }
                    VkTransformMatrixKHR transformMatrix = toVulkanMatrix(gameObjects[i]->getGameObject()->getTransform().getMatrix());
                    VkAccelerationStructureInstanceKHR instance{};
                    instance.transform = transformMatrix;
                    instance.instanceCustomIndex = instanceID;
                    instance.mask = 0xFF;
                    instance.instanceShaderBindingTableRecordOffset = 0;
                    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                    instance.accelerationStructureReference = bottomLevelASBuffers[instanceID].getDeviceAddress();
                    //std::cout<<"blas reference : "<<instance.accelerationStructureReference<<std::endl;                    
                    instances.push_back(instance);
                    totalInstancesCount++;
                }
            }
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            instancesStaggingBuffer.create(sizeof(VkAccelerationStructureInstanceKHR)*instances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            instancesStaggingBuffer.update(instances.data(), sizeof(VkAccelerationStructureInstanceKHR)*instances.size());
            instancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR)*instances.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY, 0, true);
            Buffer::copyBuffer(instancesStaggingBuffer, instancesBuffer, sizeof(VkAccelerationStructureInstanceKHR)*instances.size(), commandPool.getHandle(parentRenderer.getCurrentFrame()));
            VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
            instanceDataDeviceAddress.deviceAddress = instancesBuffer.getDeviceAddress();
            //std::cout<<"instance device adr : "<<instanceDataDeviceAddress.deviceAddress<<std::endl;
            unsigned int currentInstancesOffset = 0;
            std::deque<Buffer> scratchBuffers;
            //for (unsigned int i = 0; i < instances.size(); i++) {
            
                VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
                accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
                accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
                accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
                accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
                accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
                accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

                // Get size info
                /*
                The pSrcAccelerationStructure, dstAccelerationStructure, and mode members of pBuildInfo are ignored. Any VkDeviceOrHostAddressKHR members of pBuildInfo are ignored by this command, except that the hostAddress member of VkAccelerationStructureGeometryTrianglesDataKHR::transformData will be examined to check if it is NULL.*
                */
                VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationStructureBuildGeometryInfo.geometryCount = 1;
                accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

                uint32_t primitive_count = instances.size();

                VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                vkGetAccelerationStructureBuildSizesKHR(
                    GPUContext::instance().getDevice().getDevice(),
                    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    &accelerationStructureBuildGeometryInfo,
                    &primitive_count,
                    &accelerationStructureBuildSizesInfo);
                topLevelASBuffers.emplace_back(GPUContext::instance().getDevice());
                topLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VMA_MEMORY_USAGE_GPU_ONLY, 0,  true);

                VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                accelerationStructureCreateInfo.buffer = topLevelASBuffers.back().getHandle();
                accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                VkAccelerationStructureKHR handle;
                vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &handle);
                topLevelAS.emplace_back(handle);    
                // Create a small scratch buffer used during build of the top level acceleration structure
                scratchBuffers.emplace_back(GPUContext::instance().getDevice());
                scratchBuffers.back().create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);

                VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                accelerationBuildGeometryInfo.dstAccelerationStructure = handle;
                accelerationBuildGeometryInfo.geometryCount = 1;
                accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffers.back().getDeviceAddress();

                VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                accelerationStructureBuildRangeInfo.primitiveCount = primitive_count;
                accelerationStructureBuildRangeInfo.primitiveOffset = 0;
                accelerationStructureBuildRangeInfo.firstVertex = 0;
                accelerationStructureBuildRangeInfo.transformOffset = 0;
                std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
                vkCmdBuildAccelerationStructuresKHR(
                        commandPool.getHandle(parentRenderer.getCurrentFrame()),
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data()),
                currentInstancesOffset ++;                
            //}
            rayGenPC.tlasCount = topLevelAS.size();
            commandPool.endRecordCommandBuffer(parentRenderer.getCurrentFrame());
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandPool.getHandle(parentRenderer.getCurrentFrame());
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());                 
        }
        void RTRenderer::createDescriptorsAndPipelines() {            
            DescriptorSetLayout& rtRaygenSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 6, true); 
            rtRaygenSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtRaygenSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtRaygenSetLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtRaygenSetLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtRaygenSetLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
            rtRaygenSetLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_TLAS_STRUCTURES, VK_SHADER_STAGE_RAYGEN_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);   
            rtRaygenSetLayout.update();         
            DescriptorSetLayout& rtRayhitSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 9, true, 1); 
            rtRayhitSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR); 
            rtRayhitSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitSetLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitSetLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitsetLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_MISS_BIT_KHR);
            rtRayhitsetLayout.updateLayout(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR);
            rtRayhitsetLayout.updateLayout(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_MISS_BIT_KHR);
            rtRayhitsetLayout.updateLayout(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_MISS_BIT_KHR);
            rtRayhitSetLayout.updateLayout(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            rtRayhitSetLayout.update();
            DescriptorSetLayout& rtRayhitSpecSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 2);
            rtRayhitSpecSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            rtRayhitSpecSetLayout.update();
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT); 
            DescriptorSetLayout& rtRayhitNormalSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 3);
            rtRayhitNormalSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            rtRayhitNormalSetLayout.update(); 
            DescriptorSetLayout& rtRayhitMetSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 4);
            rtRayhitMetSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT); 
            rtRayhitMetSetLayout.update();
            DescriptorSetLayout& rtRayhitRoughSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 5);
            rtRayhitRoughSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT); 
            rtRayhitRoughSetLayout.update();
            DescriptorSetLayout& rtRayhitAOSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 6);
            rtRayhitAOSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT); 
            rtRayhitRoughSetLayout.update();
            DescriptorSetLayout& rtRayhitEmissSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 1, true, 7);
            rtRayhitEmissSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT); 
            rtRayhitEmissSetLayout.update();            
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vertexPCRange;
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(RayGenPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            pushConstants.push_back(vertexPCRange);
            GPUContext::instance().getRTPipeline(rtShader).createRTPipeline(rtShader, GPUContext::instance().getDescriptorSetLayout(rtShader), pushConstants);
            DescriptorPool& rtRaygenPool = GPUContext::instance().getDescriptorPool(rtShader, 6);
            rtRaygenPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
            rtRaygenPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            rtRaygenPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRaygenPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRaygenPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_TLAS_STRUCTURES);
            rtRaygenPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_TLAS_STRUCTURES);
            rtRaygenPool.update();
            DescriptorPool& rtRayhitPool = GPUContext::instance().getDescriptorPool(rtShader, 5, 1);
            rtRayhitPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
            rtRayhitPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
            rtRayhitPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitPool.updatePoolSize(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitPool.updatePoolSize(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitPool.updatePoolSize(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitPool.updatePoolSize(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitPool.update();
            DescriptorPool& rtRayhitSpecPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 2);
            rtRayhitSepcPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitSpecPool.update();
            DescriptorPool& rtRayhitNormalPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 3);
            rtRayhitNormalPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitNormalPool.update();
            DescriptorPool& rtRayhitMetPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 4);
            rtRayhitMetPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitMetPool.update();
            DescriptorPool& rtRayhitRoughtPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 5);
            rtRayhitRoughPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitRoughPool.update();
            DescriptorPool& rtRayhitAOPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 6);
            rtRayhitAOPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitAOPool.update();
            DescriptorPool& rtRayhitEmissPool = GPUContext::instance().getDescriptorPool(rtShader, 1, 7);
            rtRayhitEmissPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
            rtRayhitEmissPool.update();
            DescriptorSet::allocate(rtRaygenPool, rtRaygenSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 3, 1), MAX_TLAS_STRUCTURES);
            DescriptorSet::allocate(rtRayhitPool, rtRayhitSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 5, 1, 1), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitSpecPool, rtRayhitSpecSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 2), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitNormalPool, rtRayhitNormalSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 3), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitMetPool, rtRayhitMetSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 4), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitRoughPool, rtRayhitRoughSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 5), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitAOPool, rtRayhitAOSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 6), MAX_TEXTURES);
            DescriptorSet::allocate(rtRayhitEmissPool, rtRayhitEmissSetLayout, GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 7), MAX_TEXTURES);
        }
        void RTRenderer::updateDescriptorSets() {
            pc.hasGeometry = topLocalAS.size();
            bool hasLocalTLASStructure = topLocalAS.size() != 0;
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& rtRaygenSet = GPUContext::instance().getDescriptorSets(rtShader, (hasTLASStructure) ? 6 : 4, 1)[0];
            rtRaygenSet.updateBufferInfos(0, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            rtRaygenSet.updateImageInfos(1, storageImage,VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            rtRaygenSet.updateBufferInfos(2, dirLightBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRaygenSet.updateBufferInfos(3, pointLightBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);            
            if (topGlobalAS.size() != 0) {
                rtRaygenSet.updateAccelerationStructureInfos(4, topGlobalAS);
            }
            if (topLocalAS.size() != 0) {
                rtRaygenSet.updateAccelerationStructureInfos(5, topLocalAS);
            }            
            rtRaygenSet.updateDescriptorSet();
            DescriptorSet& rtRayhitSet = GPUContext::instance().getDescriptorSets(rtShader, (hasDiffuseTexture) ? 9 : 8, 1, 1)[0];
            rtRayhitSet.updateBufferInfos(0, true, GPUContext::instance().getSharedVertexBuffer(RenderTarget::VERTEX_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(1, false, GPUContext::instance().getSharedVertexBuffer(RenderTarget::VERTEX_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(2, geometryOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(3, materialBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateImageInfos(4, environmentMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            rtRayhitSet.updateImageInfos(5, frameBuffer.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            rtRayhitSet.updateImageInfos(6, cmsShadowMaps.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            rtRayhitSet.updateImageInfos(7, plShadowMaps.getTexture(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            if (hasDiffuseTexture) {
                rtRayhitSet.updateImageInfos(8, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
            }
            rtRayhitSet.updateDescriptorSet();            
            bool hasSpecTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
            if (hasSpecTexture) {
                DescriptorSet& rtRayhitSpecSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 2)[0];
                rtRayhitSpecSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitSpecSet.updateDescriptorSets();
            }     
            bool hasNormalTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
            if (hasNormalTexture) {
                DescriptorSet& rtRayhitNormalSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 3)[0];
                rtRayhitNormalSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitNormalSet.updateDescriptorSets();
            }            
            bool hasMetTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
            if (hasMetTexture) {
                DescriptorSet& rtRayhitMetSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 4)[0];
                rtRayhitMetSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitMetSet.updateDescriptorSets();
            }            
            bool hasRoughTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
            if (hasRoughTexture) {
                DescriptorSet& rtRayhitRoughSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 6)[0];
                rtRayhitRoughSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitMetSet.updateDescriptorSets();
            }            
            bool hasAOTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
            if (hasAOTexture) {
                DescriptorSet& rtRayhitAOSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 6)[0];
                rtRayhitAOSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitAOSet.updateDescriptorSets();
            }            
            bool hasEmissTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
            if (hasEmissTexture) {
                DescriptorSet& rtRayhitEmissSet = GPUContext::instance().getDescriptorSets(rtShader, 1, 1, 2)[0];
                rtRayhitEmissSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                rtRayhitEmissSet.updateDescriptorSet();
            }                        
        }
        void RTRenderer::createShaderBindingTable() {
            const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            const uint32_t groupCount = static_cast<uint32_t>(shaderGroupCount);
            const uint32_t sbtSize = shaderGroupCount * handleSizeAligned;

            std::vector<uint8_t> shaderHandleStorage(sbtSize);
            if(vkGetRayTracingShaderGroupHandlesKHR(GPUContext::instance().getDevice().getDevice(), GPUContext::instance().getRTPipeline(rtShader).getHandle(), 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to raytracing shader group handle!");
            }

         
            raygenShaderBT.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);
            raymissShaderBT.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);
            rayhitShaderBT.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR, VMA_MEMORY_USAGE_CPU_ONLY, 0, true);

            // Copy handles
            raygenShaderBT.update(shaderHandleStorage.data(), handleSize);
            raymissShaderBT.update(shaderHandleStorage.data() + handleSizeAligned, handleSize);
            rayhitShaderBT.update(shaderHandleStorage.data() + handleSizeAligned*2, handleSize);
        }
        void RTRenderer::clear () {
            parentRenderer.setTypesToRender(typesToRenderExpression, parentRenderer.getCurrentFrame());
            parentRenderer.applyCullingAndBatching();
            VkClearColorValue clearColor = {0.f, 0.f, 0.f, 0.f};
            VkImageSubresourceRange subresRange = {};
            subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subresRange.levelCount = 1;
            subresRange.layerCount = 1;
            
            vkCmdClearColorImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), storageImage[parentRenderer.getCurrentFrame()].getHandle(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
            VkMemoryBarrier memoryBarrier;
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            memoryBarrier.pNext = VK_NULL_HANDLE;
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);            
        }
        void RTRenderer::drawNextFrame() {
        }
        void RTRenderer::draw() {
            if (needToUpdateBLAS) {
                updateBLAS();
                needToUpdateBLAS = false;
            }
            if (needToUpdateTLAS) {
                updateTLAS();
                needToUpdateTLAS = false;
            }
            if (needToUpdateDescriptorSets) {
                updateDescriptorSets();
                needToUpdateDescriptorSets = false;
            }
            const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
			raygenShaderSbtEntry.deviceAddress = raygenShaderBT.getDeviceAddress();
			raygenShaderSbtEntry.stride = handleSizeAligned;
			raygenShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
			missShaderSbtEntry.deviceAddress = raymissShaderBT.getDeviceAddress();
			missShaderSbtEntry.stride = handleSizeAligned;
			missShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
			hitShaderSbtEntry.deviceAddress = rayhitShaderBT.getDeviceAddress();
			hitShaderSbtEntry.stride = handleSizeAligned;
			hitShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

			/*
				Dispatch the ray tracing commands
			*/
            std::vector<VkDescriptorSet> sets;
            for (unsigned int i = 0; i <  GPUContext::instance().getDescriptorSets(rtShader).size(); i++) {
                //std::cout<<"set : "<<linkedListSets[i][0].getHandle()<<std::endl;
                sets.push_back(GPUContext::instance().getDescriptorSets(rtShader)[i][0].getHandle());
            }
            rayGenPC.currentFrame = parentRenderer.getCurrentFrame();
			vkCmdBindPipeline(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, GPUContext::instance().getRTPipeline(rtShader).getHandle());
			vkCmdBindDescriptorSets(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, GPUContext::instance().getRTPipeline(rtShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
            vkCmdPushConstants(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), GPUContext::instance().getRTPipeline(rtShader).getLayout(), VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(RayGenPC), &rayGenPC);
			/*std::cout << "TraceRays size = " 
          << parentRenderer.getSize().x() << " x " 
          << parentRenderer.getSize().y() << "\n";*/
            vkCmdTraceRaysKHR(
				parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),
				&raygenShaderSbtEntry,
				&missShaderSbtEntry,
				&hitShaderSbtEntry,
				&callableShaderSbtEntry,
				parentRenderer.getSize().x(),
				parentRenderer.getSize().y(),
				1);
            /*uint32_t width  = parentRenderer.getSize().x();
            uint32_t height = parentRenderer.getSize().y();

            std::vector<uint8_t> cpuPixels(width * height * 4);

            for (uint32_t y = 0; y < height; y++) {
                for (uint32_t x = 0; x < width; x++) {
                    float u = float(x) / float(width);

                    cpuPixels[(y * width + x) * 4 + 0] = uint8_t(u * 255); // R
                    cpuPixels[(y * width + x) * 4 + 1] = 0;               // G
                    cpuPixels[(y * width + x) * 4 + 2] = 0;               // B
                    cpuPixels[(y * width + x) * 4 + 3] = 255;             // A
                }
            }
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {width, height, 1};
            
            staggingBuffer.create(cpuPixels.size(), 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            staggingBuffer.update(cpuPixels.data(), cpuPixels.size());
            vkCmdCopyBufferToImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),
            staggingBuffer.getHandle(),
            storageImage[parentRenderer.getCurrentFrame()].getHandle(),
            VK_IMAGE_LAYOUT_GENERAL,
            1,
            &region);*/
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.image = parentRenderer.getRenderingImage().getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.image = storageImage[parentRenderer.getCurrentFrame()].getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }
            
            VkImageCopy copyRegion{};
			copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { parentRenderer.getSize().x(), parentRenderer.getSize().y(), 1 };
			
            vkCmdCopyImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), storageImage[parentRenderer.getCurrentFrame()].getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, parentRenderer.getRenderingImage().getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			{
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = parentRenderer.getRenderingImage().getLayout();
                barrier.image = parentRenderer.getRenderingImage().getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.image = storageImage[parentRenderer.getCurrentFrame()].getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }
        }
    }    
}