namespace odfaeg {
    namespace graphic {
        void RTRenderer::updateBLAS() { 
            instancesGroupCount = 0; 
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
                            AccelerationStructure bottomLevelAS; 
                            createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);
                            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                            accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
                            accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                            accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

                            // Create a small scratch buffer used during build of the bottom level acceleration structure
                            RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

                            VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                            accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                            accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                            accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                            accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                            accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
                            accelerationBuildGeometryInfo.geometryCount = 1;
                            accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                            accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

                            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                            accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                            accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset;
                            accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset;
                            accelerationStructureBuildRangeInfo.transformOffset = 0;
                            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                            
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
                        accelerationStructureGeometry.geometry.triangles.transformData = nullptr;
                        
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
                        AccelerationStructure bottomLevelAS; 
                        createAccelerationStructureBuffer(bottomLevelAS, accelerationStructureBuildSizesInfo);
                        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                        accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                        accelerationStructureCreateInfo.buffer = bottomLevelAS.buffer;
                        accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                        accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &bottomLevelAS.handle);

                        // Create a small scratch buffer used during build of the bottom level acceleration structure
                        RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

                        VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                        accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                        accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                        accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                        accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                        accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
                        accelerationBuildGeometryInfo.geometryCount = 1;
                        accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                        accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

                        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                        accelerationStructureBuildRangeInfo.primitiveCount = numTriangles;
                        accelerationStructureBuildRangeInfo.primitiveOffset = sm.indexOffset;
                        accelerationStructureBuildRangeInfo.firstVertex = sm.vertexOffset;
                        accelerationStructureBuildRangeInfo.transformOffset = 0;
                        std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };                      
                        
                        GeometryOffset geometryOffset;
                        geometryOffset.vertexOffset = sm.vertexOffset;
                        geometryOffset.indexOffset = sm.indexOffset + lods[0].indexOffset;                        
                        geometryOffsets.push_back(geometryOffset);                        
                        singleInstancesCount++;                         
                    } 
                } 
            }            
        }
        void RTRenderer::updateTLAS() {
            std::vector<std::vector<VkAccelerationStructureInstanceKHR>> instances;
            unsigned int singleInstancesIndex = 0;
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
                    instance.accelerationStructureReference = bottomLevelASs[blasIndex].deviceAddress;
                    instancesId.resize(instancesId+1);
                    instances[instancesId].push_back(instance);
                }
            } 
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

                uint32_t primitive_count = 1;

                VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
                accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
                vkGetAccelerationStructureBuildSizesKHR(
                    vkDevice.getDevice(),
                    VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                    &accelerationStructureBuildGeometryInfo,
                    &primitive_count,
                    &accelerationStructureBuildSizesInfo);

                createAccelerationStructureBuffer(topLevelAS, accelerationStructureBuildSizesInfo);

                VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
                accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
                accelerationStructureCreateInfo.buffer = topLevelAS.buffer;
                accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
                accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                vkCreateAccelerationStructureKHR(vkDevice.getDevice(), &accelerationStructureCreateInfo, nullptr, &topLevelAS.handle);

                // Create a small scratch buffer used during build of the top level acceleration structure
                RayTracingScratchBuffer scratchBuffer = createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

                VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
                accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
                accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
                accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
                accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
                accelerationBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
                accelerationBuildGeometryInfo.geometryCount = 1;
                accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
                accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

                VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
                accelerationStructureBuildRangeInfo.primitiveCount = 1;
                accelerationStructureBuildRangeInfo.primitiveOffset = i;
                accelerationStructureBuildRangeInfo.firstVertex = 0;
                accelerationStructureBuildRangeInfo.transformOffset = 0;
                std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
            }
        }
    }
}