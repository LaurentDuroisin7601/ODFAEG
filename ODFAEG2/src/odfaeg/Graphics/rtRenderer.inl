namespace odfaeg {
    namespace graphic {
        RTRenderer::RTRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId = -1, bool usethread=true) :
        parentRenderer(parentRenderer),
        rtShader(GPUContext::instance().getDeive()) {
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
            VkPhysicalDeviceProperties2 deviceProperties2{};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(GPUContext::instance().getDevice().getPhysicalDevice(), &deviceProperties2);

            // Get acceleration structure properties, which will be used later on in the sample
            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            VkPhysicalDeviceFeatures2 deviceFeatures2{};
            deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures2.pNext = &accelerationStructureFeatures;
            vkGetPhysicalDeviceFeatures2(GPUContext::instance().getDevice().getPhysicalDevice(), &deviceFeatures2);
            math::Vector2u size = parentRenderer.getSize();
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                storageImage.emplace_back(GPUContext::instance().getDevice());
                storageImage.back().createImage(size.x(), size.y(), 1, VK_IMAGE_TYPE_2D, parentRenderer.getImageFormat(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                        1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
                headPtrsStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, parentRenderer.getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
            }            
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                ubo.emplace_back(GPUContext::instance().getDevice());
                ubo.back().create(sizeof(UBODatas), VK_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                ubo.back().update(uboDatas.data(), sizeof(UBOData));  
            }
            std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";
            if (!rtShader.loadRaytracingFromFileSpv(shaderDir + "/raygenShader.raygen", shaderDir + "/raymissShader.raymiss", shaderDir+"/rayhitShader.rayhit")) {
                throw std::runtime_error("Could not load env map shader");
            }
            shaderGroupCount = 3;
            createDescriptorAndPipelines();
            createShaderBindingTable(); 
        } 
        void RTRenderer::createCommandPool() {
            Device::QueueFamilyIndices queueFamilyIndices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
            commandPool.create(queueFamilyIndices.graphicsFamily.value());
            commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
        }
        void RTRenderer::updateBLAS() { 
            VkTransformMatrixKHR transformMatrix = {
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f
            };
            transformMatrixStaggingBuffer.create(sizeof(VkTransformMatrixKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);         
            transformMatrixStaggingBuffer.update(&transformMatrix, sizeof(VkTransformMatrixKHR))
            transformMatrixBuffer.create(sizeof(VkTransformMatrixKHR), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            Buffer::copyBuffer(transformMatrixStaggingBuffer, transformMatrixBuffer, sizeof(VkTransformMatrixKHR), commandPool.getHandle(parentRenderer.getHandle()));
            std::vector<VkTransformMatrixKHR> transformMatrices;
            instancesGroupCount = 0; 
            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

            vertexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)[entity::Triangles].getVertexBuffer().getDeviceAddress();
            indexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)[entity::Triangles].getIndexBuffer().getDeviceAddress();
            transformBufferDeviceAddress.deviceAddress = transformBuffer.getDeviceAddress();
            std::deque<Material*> materials = Material::getAllMaterials();
            std::vector<MaterialData> materialDatas;
            //std::cout<<"material : "<<materials.size()<<std::endl;
            for (unsigned int i = 0; i < materials.size(); i++) {
                //std::cout<<"texture id : "<<materials[i]->getTexture(entity::SubMesh::DIFFUSE)->getId()<<std::endl;
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
                
                material.instanceGroupId = materials[i]->getInstanceGroup();
                material.reflectable = (materials[i]->isReflectable()) ? 1 : 0;
                material.refractable = (materials[i]->isRefractable()) ? 1 : 0;
                /*std::cout<<"id : "<<material.materialId<<", reflectable : "<<material.reflectable<<"refractable : "<<material.refractable<<std::endl;
                system("PAUSE");*/
                /*if (material.instanceGroupId != -1)
                    system("PAUSE");**/
                materialDatas.push_back(material);

            }
            materialStaggingBuffer.create(sizeof(Material)*materials.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);         
            materialStaggingBuffer.update(&transformMatrix, sizeof(Material)*materials.size())
            materialBuffer.create(ssizeof(Material)*materials.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
            
            Buffer::copyBuffer(materialStaggingBuffer, materialBuffer, sizeof(Material)*materials.size(), commandPool.getHandle(parentRenderer.getHandle()));
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    entity::SubMesh sm = gameObjects[i]->getGameObject()->getSubMeshes()[j];
                    //Ensuite on parcours les matériaux à plusieurs instances.
                    if (!gameObjects[i]->getMaterials()[0]->getInstanceGroupId != -1) {
                        if (gameObjects[i]->getMaterials()[0]->materialSet == 0) {
                            gameObjects[i]->getMaterials()[0]->materialSet == 1;
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
                            accelerationStructureGeometry.geometry.triangles.transformData = transformBufferAddress;
                            
                            VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                            accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                            accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                            accelerationStructureBuildGeometryInfo.geometryCount = 1;
                            accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                            std::array<VertexArray::LODLevel, 5> lods = sm.getVertexArray().getLods();
                            const uint32_t numTriangles = lods[0].indexCount / 3;
                            VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                            accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                            vkGetAccelerationStructureBuildSizesKHR(
                            GPUContext::instance().getDevice(),
                            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                            &accelerationStructureBuildGeometryInfo,
                            &numTriangles,
                            &accelerationStructureBuildSizesInfo);
                            bottomLevelASBuffers.emplace_back(GPUContext::instance().getDevice()); 
                            bottomLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
                            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                            accelerationStructureCreateInfo.buffer = bottomLevelASBuffes.back().getHandel();
                            accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelASBuffers.back().getHandle());

                            // Create a small scratch buffer used during build of the bottom level acceleration structure
                            Buffer scratchBuffer.create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);


                            VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                            accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                            accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                            accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                            accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelASBuffers.back().getHandle();
                            accelerationBuildGeometryInfo.geometryCount = 1;
                            accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                            accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getDeviceAddress();

                            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                            accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                            accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset;
                            accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset;
                            accelerationStructureBuildRangeInfo.transformOffset = 0;
                            
                            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                            vkCmdBuildAccelerationStructuresKHR(
                            commandPool.getHandle(commandPool.getHandle(parentRenderer.getCurrentFrame()));
                            1,
                            &accelerationBuildGeometryInfo,
                            accelerationBuildStructureRangeInfos.data());
                            
                            GeometryOffset geometryOffset;
                            geometryOffset.vertexOffset = sm.vertexOffset;
                            geometryOffset.indexOffset = sm.indexOffset + lods[0].indexOffset;   
                            geometryOffset.materialOffset = submesh.materialId;
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
                    if (!gameObjects[i]->getMaterials()[0]->getInstanceGroupId == -1) {
                    
                        
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
                        accelerationStructureGeometry.geometry.triangles.transformData = transformBufferAddress;
                        
                        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
                        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                        accelerationStructureBuildGeometryInfo.geometryCount = 1;
                        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                        std::array<VertexArray::LODLevel, 5> lods = sm.getVertexArray().getLods();
                        const uint32_t numTriangles = lods[0].indexCount / 3;
                        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                        vkGetAccelerationStructureBuildSizesKHR(
                        GPUContext::instance().getDevice(),
                        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                        &accelerationStructureBuildGeometryInfo,
                        &numTriangles,
                        &accelerationStructureBuildSizesInfo);
                        bottomLevelASBuffers.emplace_back(GPUContext::instance().getDevice()); 
                        bottomLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
                        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                        accelerationStructureCreateInfo.buffer = bottomLevelASBuffes.back().getHandel();
                        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelASBuffers.back().getHandle());

                        // Create a small scratch buffer used during build of the bottom level acceleration structure
                        Buffer scratchBuffer.create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);


                        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                        accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelASBuffers.back().getHandle();
                        accelerationBuildGeometryInfo.geometryCount = 1;
                        accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getDeviceAddress();

                        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                        accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                        accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset;
                        accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset;
                        accelerationStructureBuildRangeInfo.transformOffset = 0;
                        
                        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                        vkCmdBuildAccelerationStructuresKHR(
                        commandPool.getHandle(commandPool.getHandle(parentRenderer.getCurrentFrame()));
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data());
                        
                        GeometryOffset geometryOffset;
                        geometryOffset.vertexOffset = sm.vertexOffset;
                        geometryOffset.indexOffset = sm.indexOffset + lods[0].indexOffset;
                        geometryOffset.materialOffset = submesh.materialId;                        
                        geometryOffsets.push_back(geometryOffset);                        
                        singleInstancesCount++;                         
                    } 
                } 
            }            
            VkDeviceSize bufferSize = geometryOffsets.size() * sizeof(GeometryOffset);
            geometryOffsetStaggingBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);    
            geometryOffsetStaggingBuffer.update(geometryOffsets.data(), bufferSize);         
            geometryOffsetBuffer.create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            Buffer::copyBuffer(geometryOffsetStaggingBuffer, geometryOffsetBuffer, deviceSize, commandPool.getHandle(parentRenderer.getHandel()));
            commandPool.endRecordCommandBuffer(parentRenderer.getCurrentBuffer());
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = commandPool.getHandles().size();
            submitInfo.pCommandBuffers = commandPool.getHandles().data();
            Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice(), VK_NULL_HANDLE);
            if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
                throw std::runtime_error("Echec de l'envoi d'un command buffer!");
            }
            vkDeviceWaitIdle(GPUContext::instance().getDevice().getDevice());                 
        }
        void RTRenderer::updateTLAS() {
            std::vector<std::vector<VkAccelerationStructureInstanceKHR>> instances;
            unsigned int singleInstancesIndex = 0;
            instancesId.resize(singleInstancesCount+instancesGroupCount+1);
            unsigned int totalInstancesCount = 0;
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    unsigned int instancesID;
                    if (gameObjects[i]->getMaterials()[0]->getInstanceGroupId() == -1) {
                        instancesId = singleInstancesIndex + instancesGroupCount;
                        singleInstancesIndex++;
                    } else {
                        instancesId = gameObjects[i]->getMaterials()[0]->getInstanceGroupId();
                    }
                    VkTransformMatrixKHR transformMatrix = toVulkanMatrix(gameObjects[i]->getGameObject()->getTransformMatrix().getMatrix());
                    VkAccelerationStructureInstanceKHR instance{};
                    instance.transform = transformMatrix;
                    instance.instanceCustomIndex = instanceId;
                    instance.mask = 0xFF;
                    instance.instanceShaderBindingTableRecordOffset = 0;
                    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
                    instance.accelerationStructureReference = bottomLevelASBuffers[instanceID].getDeviceAddress();                    
                    instances[instancesId].push_back(instance);
                    totalInstancesCount++;
                }
            }
            staggingInstancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR)*totalInstancesCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            staggingInstancesBuffer.update(instances.data(), sizeof(VkAccelerationStructureInstanceKHR)*totalInstancesCount);
            instancesBuffer.back().create(sizeof(VkAccelerationStructureInstanceKHR)*totalInstancesCount, VK_BUFFER_USAGE_STORAGE_BUFFER | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
            Buffer::copyBuffer(staggingInstanceBuffer, instancesBuffer.back(), sizeof(VkAccelerationStructureInstanceKHR)**totalInstancesCount, parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()));
            VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
            instanceDataDeviceAddress.deviceAddress = instanceBuffer.back().getDeviceAddress();
            unsigned int currentInstancesOffset = 0;
            for (unsigned int i = 0; i < instances.size(); i++) {
            
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

                uint32_t primitive_count = instances[i].size();

                VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                vkGetAccelerationStructureBuildSizesKHR(
                    GPUContext::instance().getDevice().getDevice(),
                    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    &accelerationStructureBuildGeometryInfo,
                    &primitive_count,
                    &accelerationStructureBuildSizesInfo);
                topLevelASBuffers.emplace_back(GPUContext::instance().getDevice());
                topLevelASBuffers.back().create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);

                VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                accelerationStructureCreateInfo.buffer = topLevelASBuffers.back().getHandle();
                accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                vkCreateAccelerationStructureKHR(GPUContext::instance().getDevice().getDevice(), &accelerationStructureCreateInfo, nullptr, &topLevelASBuffers.back().getHandle());

                // Create a small scratch buffer used during build of the top level acceleration structure
                Buffer scratchBuffer.create(accelerationStructureBuildSizesInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);

                VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelASBuffers.getHandle();
                accelerationBuildGeometryInfo.geometryCount = 1;
                accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getDeviceAddress();

                VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                accelerationStructureBuildRangeInfo.primitiveCount = primitiveCount;
                accelerationStructureBuildRangeInfo.primitiveOffset = currentInstancesOffset;
                accelerationStructureBuildRangeInfo.firstVertex = 0;
                accelerationStructureBuildRangeInfo.transformOffset = 0;
                std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
                vkCmdBuildAccelerationStructuresKHR(
                        parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data());
                currentInstancesOffset += instances[i].size();
            }
        }
        void RTPipeline::createDescriptorsAndPipelines() {            
            DescriptorSetLayout& rtRaygenSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 3, true); 
            rtSetRaygenSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtSetRaygenSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            rtSetRaygenSetLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_TLAS_STRUCTURES, VK_SHADER_STAGE_RAYGEN_BIT_KHR, , VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);            
            DescriptorSetLayout& rtRayhitSetLayout = GPUContext::instance().getDescriptorSetLayout(rtShader, 5, true, 1); 
            rtRayhitSetLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR); 
            rtRayhitSetLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitSetLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitSetLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            rtRayhitSetLayout.updateLayout(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, , VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
            rtRayhitSetLayout.update();
            std::vector<VkPushConstantRange> pushConstants;
            VkPushConstantRange vertexPCRange;
            vertexPCRange.offset = 0;
            vertexPCRange.size = sizeof(RayGenPC);
            vertexPCRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            pushConstants.push_back(vertexPCRange);
            GPUContext::instance().getRTPipeline(rtShader).createRTPipeline(rtShader, GPUContext::instance().getDescriptorSetLayout(rtShader), pushConstants);
            DescriptorPool& rtRaygenPool = GPUContext::instance().getDescriptorPool(rtShader, 3);
            rtRaygenPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
            rtRaygenPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, MAX_FRAMES_IN_FLIGHT);
            rtRaygenPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, MAX_TLAS_STRUCTURES);
            rtRaygenPool.update();
            DescriptorPool& rtRayhitPool = GPUContext::instance().getDescriptorPool(rtShader, 5, 1);
            rtRayhitPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
            rtRayhitPool.updatePoolSize(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLE, MAX_TEXTURES);
            rtRayhitPool.update();
            DescriptorSet::allocate(rtRaygenPool, rtRaygenLayout, GPUContext::instance().getDescriptorSets(rtShader, 3, 1), MAX_TLAS_STRUCTURES);
            DescriptorSet::allocate(rtRayhitPool, rtRayhitLayout, GPUContext::instance().getDescriptorSets(rtShader, 5, 1, 1), MAX_TEXTURES);
        }
        void RTPipeline::updateDescriptorSets() {
            bool hasTLASStructure = topLevelADBuffers.size() != 0;
            bool hasDiffuseTexture = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
            DescriptorSet& rtRaygenSet = GPUContext::instance().getDescriptorSets(rtShader, (hasTLASStrcture) ? 3 : 2, 1)[0];
            rtRaygenSet.updateBufferInfos(0, ubos, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            rtRaygenSet.updateBufferInfos(0, storageImage,VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
            if (hasTLASStructure) {
                rtRaygenSet.updateBufferInfos(0, topLevelASBuffers, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
            }
            DescriptorSet& rtRayhitSet = GPUContext::instance().getDescriptorSets(rtShader, (hasDiffuseTexture) ? 5 : 4, 1)[0];
            rtRayhitSet.updateBufferInfos(0, true, GPUContext::instance().getSharedVertexBuffers(RenderTarget::VERTEX_BUFFER)[entity::Triangles], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(1, false, GPUContext::instance().getSharedVertexBuffers(RenderTarget::VERTEX_BUFFER)[entity::Triangles], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(2, geometryOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            rtRayhitSet.updateBufferInfos(3, GPUContext::instance().getSharedBuffers(RenderTarget::MATERIAL_DATA_BUFFER), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            if (hasDiffuseTexture) {
                rtRayhitSet.updatePoolSize(4, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLE);
            }
        }
        void RTPipeline::createShaderBindingTable() {
            const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
            const uint32_t handleSizeAligned = (rayTracingPipelineProperties.shaderGroupHandleSize + rayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(rayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
            const uint32_t sbtSize = groupCount * handleSizeAligned;

            std::vector<uint8_t> shaderHandleStorage(sbtSize);
            if(vkGetRayTracingShaderGroupHandlesKHR(vkDevice.getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()) != VK_SUCCESS) {
                throw std::runtime_error("failed to raytracing shader group handle!");
            }

            const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            const VkMemoryPropertyFlags memoryUsageFlags = VMA_MEMORY_USAGE_CPU_ONLY;
            rayGenShaderBT.create(handleSize, bufferUsageFlags, memoryUsageFlags);
            raymissShaderBT.create(handleSize, bufferUsageFlags, memoryUsageFlags);
            rayhitShaderBT.create(handleSize, bufferUsageFlags, memoryUsageFlags);

            // Copy handles
            rayGenShaderBT.update(shaderHandleStorage.data(), handleSize);
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
            
            vkCmdClearColorImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), storageImage.back().getHandle(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &subresRange);
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
			raygenShaderSbtEntry.deviceAddress = raygenShaderBindingTable.getDeviceAddress();
			raygenShaderSbtEntry.stride = handleSizeAligned;
			raygenShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
			missShaderSbtEntry.deviceAddress = missShaderBindingTable.getDeviceAddress();
			missShaderSbtEntry.stride = handleSizeAligned;
			missShaderSbtEntry.size = handleSizeAligned;

			VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
			hitShaderSbtEntry.deviceAddress = hitShaderBindingTable.getDeviceAddress();
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
			vkCmdBindPipeline(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, GPUContext::instance().getRtPipeline(rtShader).getHandle());
			vkCmdBindDescriptorSets(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, GPUContext::instance().getRtPipeline(rtShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
            vkCmdPushConstants(envMapCmdPools[cmp].getHandle(envMap.getCurrentFrame()), GPUContext::instance().getRTPipeline(rtShader).getLayout(), VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(RayGenPC), &raygenPC);
			vkCmdTraceRaysKHR(
				parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),
				&raygenShaderSbtEntry,
				&missShaderSbtEntry,
				&hitShaderSbtEntry,
				&callableShaderSbtEntry,
				parentRenderer.getSize().x(),
				parentRenderer.getSize().y(),
				1);
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
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.image = storageImage.back().getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }

            VkImageCopy copyRegion{};
			copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.srcOffset = { 0, 0, 0 };
			copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
			copyRegion.dstOffset = { 0, 0, 0 };
			copyRegion.extent = { parentRenderer.getRenderingImage().getSize().x(), parentRenderer.getRenderingImage().getSize().y(), 1 };
			vkCmdCopyImage(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), storageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, window.getSwapchainImages()[window.getImageIndex()], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			{
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier.image = parentRenderer.getRenderingImage().getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
                barrier.image = storageImage.back.getHandle();
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.layerCount = 1;
                vkCmdPipelineBarrier(parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            }
        }
    }    
}