namespace odfaeg {
    namespace graphic {
        RTRenderer::RTRenderer(RenderTarget& parentRenderer, unsigned int layer, std::string typesToRenderExpression, int windowId = -1, bool usethread=true) :
        parentRenderer(parentRenderer) {
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
            storageImage.emplace_back(GPUContext::instance().getDevice());
            storageImage.back().createImage(size.x(), size.y(), 1, VK_IMAGE_TYPE_2D, parentRenderer.getImageFormat(), VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY,
                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL);
            headPtrsStorageImage.back().createImageView(VK_IMAGE_VIEW_TYPE_2D, parentRenderer.getImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1, 1);
            ubo.emplace_back(GPUContext::instance().getDevice());
            for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                ubo.back().create(sizeof(UBODatas), VK_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
                ubo.back().update(uboDatas.data(), sizeof(UBOData));  
            }
        }   
        void RTRenderer::createDescriptorAndPipelines() {
            

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
            transformMatrixBufferStaggingBuffer.create(sizeof(VkTransformMatrixKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);         
            transformMatrixBuffer.create(sizeof(VkTransformMatrixKHR), , VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
            commandPool.beginRecordCommandBuffer(parentRenderer.getCurrentFrame());
            Buffer::copyBuffer(transformMatrixStaggingBuffer, transformMatrixBuffer, sizeof(VkTransformMatrixKHR), commandPool.getHandle(parentRenderer.getHandel()));
            std::vector<VkTransformMatrixKHR> transformMatrices;
            instancesGroupCount = 0; 
            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
            VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

            vertexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER).getVertexBuffer().getDeviceAddress();
            indexBufferDeviceAddress.deviceAddress = GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER).getIndexBuffer().getDeviceAddress();
            transformBufferDeviceAddress.deviceAddress = transformBuffer.getDeviceAddress();
            for (unsigned int i = 0; i < gameObjects.size(); i++) {
                for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
                    entity::SubMesh sm = gameObjects[i]->getGameObject()->getSubMeshes()[j];
                    //Ensuite on parcours les matériaux à plusieurs instances.
                    if (!gameObjects[i]->getMaterials()[0]->getInstanceGroupId != -1)
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
                        geometryOffsets.push_back(geometryOffset);                        
                        singleInstancesCount++;                         
                    } 
                } 
            }
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
            instancesId.resize(singleInstancesCount+instancesId+1);
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
                }
            } 
            std::vector<uint32_t> instancesOffsets;
            std::vector<VkAccelerationStructureInstanceKHR> bInstances;
            unsigned int currentOffset = 0;
            for (unsigned int i = 0; i < instances.size(); i++) {
                InstanceData instancesData;
                instanceOffsets.offset = currentOffset; 
                instancesOffsets.push_back(currentOffset);
                currentOffset += instances[i].size();                
                bInstances.insert(bInstances.end(), instances[i].begin(), instances[i].end());                
            }
            staggingInstancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR)*bInstances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
            staggingInstancesBuffer.update(bInstances.data(), sizeof(VkAccelerationStructureInstanceKHR)*bInstances.size());
            instancesBuffer.back().create(sizeof(VkAccelerationStructureInstanceKHR)*bInstances.size(), VK_BUFFER_USAGE_STORAGE_BUFFER | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VMA_ALLOCATION_CREATE_DEVICE_ADDRESS_BIT);
            Buffer::copyBuffer(staggingInstanceBuffer, instancesBuffer.back(), sizeof(VkAccelerationStructureInstanceKHR)*bInstances.size(), parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()));
            VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
            instanceDataDeviceAddress.deviceAddress = instanceBuffer.back().getDeviceAddress();
            for (unsigned int i = 0; i < instancesOffsets.size(); i++) {
            
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

                uint32_t primitive_count = 1;

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
                accelerationStructureBuildRangeInfo.primitiveCount = 1;
                accelerationStructureBuildRangeInfo.primitiveOffset = instancesDatas[i].offset;
                accelerationStructureBuildRangeInfo.firstVertex = 0;
                accelerationStructureBuildRangeInfo.transformOffset = 0;
                std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
                vkCmdBuildAccelerationStructuresKHR(
                        parentRenderer.getCommandPool().getHandle(parentRenderer.getCurrentFrame()),
                        1,
                        &accelerationBuildGeometryInfo,
                        accelerationBuildStructureRangeInfos.data());
            }
        }
    }
}