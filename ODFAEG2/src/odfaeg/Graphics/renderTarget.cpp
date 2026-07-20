module;
#include <deque>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "../../../include/odfaeg/config.hpp"
#include <iostream>
#include <vector>
#include <odfaeg/config.hpp>
#include <mutex>
//import odfaeg.graphic.renderTarget;
module odfaeg.graphic.renderTarget;
import odfaeg.core.utilities;
import odfaeg.graphic.material;
import odfaeg.entity.entity;
import odfaeg.graphic.viewportMatrix;
import odfaeg.math.vec;
import odfaeg.physic.boundingBox;
import odfaeg.entity.vertex;
import odfaeg.graphic.texture;
import odfaeg.graphic.gpuContext;
import odfaeg.graphic.particleSystemUpdater;
import odfaeg.graphic.morphAnimUpdater;
import odfaeg.graphic.boneAnimUpdater;
import odfaeg.graphic.renderStates;
import odfaeg.graphic.blendMode;
import odfaeg.entity.gameObject;
import odfaeg.entity.vertexArray;
namespace odfaeg {
	namespace graphic {

		RenderTarget::RenderTarget(Device& device, bool enableDepthTest, bool enableStencilTest) : enableDepthTest(enableDepthTest), enableStencilTest(enableStencilTest), device(device), commandPool(device), computeCommandPool(device), defaultRenderingShader(device), cullingBatchingShader(device), resetBuffersShader(device), vertexBufferShader(device), depthStencilTexture(device),
		gameObjects(getGameObjects()), objects(GPUContext::instance().getSharedBuffers(OBJECT_BUFFER)), staggingObjects(GPUContext::instance().getSharedBuffers(OBJECT_STAGGING_BUFFER)),
		subMeshes(GPUContext::instance().getSharedBuffers(SUBMESHES_BUFFER)), staggingSubMeshes(GPUContext::instance().getSharedBuffers(SUBMESHES_STAGGING_BUFFER)),
		lodLevel(GPUContext::instance().getSharedBuffers(LOD_BUFFER)), staggingLODLevel(GPUContext::instance().getSharedBuffers(LOD_STAGGING_BUFFER)),
		modelDatas(GPUContext::instance().getSharedBuffers(MODEL_DATA_BUFFER)), staggingModelDatas(GPUContext::instance().getSharedBuffers(STAGGING_MODEL_DATA_BUFFER)),
		materialDatas(GPUContext::instance().getSharedBuffers(MATERIAL_DATA_BUFFER)), staggingMaterialDatas(GPUContext::instance().getSharedBuffers(STAGGING_MATERIAL_DATA_BUFFER)),
		vertices(GPUContext::instance().getSharedVertexBuffer(VERTEX_BUFFER)),
		outputMeshes(GPUContext::instance().getSharedBuffers(OUTPUT_MESHES+registeredRenderTargets.size()*NB_BUFFERS)),
		outputModelDatas(GPUContext::instance().getSharedBuffers(OUTPUT_MODELS+registeredRenderTargets.size()*NB_BUFFERS)),
		outputMaterialDatas(GPUContext::instance().getSharedBuffers(OUTPUT_MATERIALS+registeredRenderTargets.size()*NB_BUFFERS))		
		{
			
			id = registeredRenderTargets.size();
			//std::cout<<"id : "<<id<<",output model id : "<<OUTPUT_MODELS+registeredRenderTargets.size()*NB_BUFFERS<<std::endl;			
			registeredRenderTargets.push_back(this);			
			
		}
		int RenderTarget::getId() {
			return id;
		}
		unsigned int RenderTarget::getNbRenderTarget() {
			return registeredRenderTargets.size();
		}
		void RenderTarget::addGameObject(Mesh* gameObject) {
			for (unsigned int i = 0; i < gameObject->getChildren().size(); i++) {
				addGameObject(gameObject->getChildren()[i]);
			}
			gameObjects.push_back(gameObject);
			/*if (gameObject->getChildren().size() > 0)
				std::cout<<"children = "<<gameObject->getChildren().size()<<std::endl;*/
			/*for (unsigned int i = 0; i < gameObject->getGameObject()->getChildren().size(); i++) {
				addGameObject(gameObject->getGameObject()->getChildren()[i]);
			}*/
			needToUpdateBuffers = true;
			/*ParticleSystemUpdater::instance().setReady(false);
			MorphAnimUpdater::instance().setReady(false);*/
		}
		void RenderTarget::removeGameObject(Mesh* gameObject) {
			std::vector<Mesh*>::iterator it;
			for (it = gameObjects.begin(); it != gameObjects.end();) {
				if (*it == gameObject) {
					it = gameObjects.erase(it);
				} else {
					it++;
				}
			}
			needToUpdateBuffers = true;
		}
		RenderTarget::~RenderTarget() {
			//std::cout<<"destory buffers"<<std::endl;
			std::vector<RenderTarget*>::iterator it;
			for (it = registeredRenderTargets.begin(); it != registeredRenderTargets.end();) {
				if (*it == this) {
					it = registeredRenderTargets.erase(it);
				} else {
					it++;
				}
			}
		}
		void RenderTarget::initialize() {
			vkCmdDrawMeshTasksEXT =
    		(PFN_vkCmdDrawMeshTasksEXT) vkGetDeviceProcAddr(device.getDevice(), "vkCmdDrawMeshTasksEXT");
			if (!vkCmdDrawMeshTasksEXT) {
				std::cerr << "vkCmdDrawMeshTasksEXT is NULL (extension VK_EXT_mesh_shader pas dispo / pas activée)" << std::endl;
				// Ne JAMAIS appeler la fonction dans ce cas
				system("PAUSE");
			}
			//std::cout<<"initialize"<<std::endl;
			//std::cout<<"size : "<<getSize().x()<<','<<getSize().y()<<std::endl;
			//std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
			m_defaultCamera = Camera(getSize().x(),getSize().y(), -1, 1);
			//m_defaultCamera.setViewport(physic::BoundingBox(0, 0, -getSize().y() - 200, getSize().x(), getSize().y(), getSize().y() + 200));
			
			//std::cout<<"default camera : "<<m_defaultCamera.getSize()<<','<<m_defaultCamera.getViewport().getPosition()<<','<<m_defaultCamera.getViewport().getSize()<<std::endl;
			std::string shaderDir = std::string(ODFAEG_INSTALL_DIR) + "/Shader";			
			if (!resetBuffersShader.loadFromFile(shaderDir+"/resetBuffers.comp")) {
				throw std::runtime_error("Failed to compile reset buffers shader!");
			}
			if (device.areMeshShadersSupported()) {
				if (!cullingBatchingShader.loadFromFile(shaderDir+"/meshCullingBatching.comp")) {
					throw std::runtime_error("Failed to compile culling batching shader!");
				}
				if (!defaultRenderingShader.loadMeshFromFileSpv(shaderDir+"/meshShader.mesh.spv", shaderDir+"/meshRenderTarget.frag.spv", shaderDir+"/taskShader.task.spv")) {
					throw std::runtime_error("Failed to compile render target default shader!");
				}
				if (!vertexBufferShader.loadFromFile(shaderDir+"/vertexBuffer.vert", shaderDir+"/vertexBuffer.frag")) {
					throw std::runtime_error("Failed to compile vertex buffer shader!");
				}
			} else {
				if (!cullingBatchingShader.loadFromFile(shaderDir+"/cullingBatching.comp")) {
					throw std::runtime_error("Failed to compile culling batching shader!");
				}
				if (!defaultRenderingShader.loadFromFile(shaderDir+"/renderTarget.vert", shaderDir+"/renderTarget.frag")) {
					throw std::runtime_error("Failed to compile render target default shader!");
				}
				if (!vertexBufferShader.loadFromFile(shaderDir+"/vertexBuffer.vert", shaderDir+"/vertexBuffer.frag")) {
					throw std::runtime_error("Failed to compile vertex buffer shader!");
				}
			}
			Device::QueueFamilyIndices queueFamilyIndices = device.findQueueFamilies(device.getPhysicalDevice(), getSurface());
			commandPool.create(queueFamilyIndices.graphicsFamily.value());
			computeCommandPool.create(queueFamilyIndices.computeFamily.value());
			commandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
			//std::cout<<"comand buffers : "<<commandPool.getHandle(0)<<','<<commandPool.getHandle(1)<<std::endl;
			computeCommandPool.createCommandBuffers(true, MAX_FRAMES_IN_FLIGHT);
			for (unsigned int i = 0; i < scissors.size(); i++) {
				scissors[i].offset = { 0, 0 };
				scissors[i].extent = getExtents();
			}
			for (unsigned int i = 0; i < viewports.size(); i++) {
				viewports[i].x = m_camera.getViewport().getPosition().x();
				viewports[i].y = m_camera.getViewport().getPosition().y();
				viewports[i].width = m_camera.getViewport().getSize().x();
				viewports[i].height = m_camera.getViewport().getSize().y();
				viewports[i].minDepth = 0.0f;
				viewports[i].maxDepth = 1.0f;
			}
			if (staggingObjects.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingObjects.emplace_back(device);
				}
			}	
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				staggingObjectTypes.emplace_back(device);
			}
			if (staggingSubMeshes.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingSubMeshes.emplace_back(device);
				}
			}
			if (staggingMeshlets.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingMeshlets.emplace_back(device);
				}
			}
			if (staggingLODLevel.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingLODLevel.emplace_back(device);
				}
			}
			if (staggingModelDatas.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingModelDatas.emplace_back(device);
				}
			}
			if (staggingMaterialDatas.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					staggingMaterialDatas.emplace_back(device);
				}
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				objectTypes.emplace_back(device);
				objectTypes.back().create(sizeof(int), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			if (objects.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					objects.emplace_back(device);
					objects.back().create(sizeof(Object),  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			if (subMeshes.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					subMeshes.emplace_back(device);
					subMeshes.back().create(sizeof(entity::SubMesh), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			if (lodLevel.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					lodLevel.emplace_back(device);
					lodLevel.back().create(sizeof(entity::VertexArray::LODLevel) * 5, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			//std::cout<<"load level ok"<<std::endl;
			if (modelDatas.empty()) {
				for (unsigned int i = 0; i < 1; i++) {
					modelDatas.emplace_back(device);
					modelDatas.back().create(sizeof(ModelData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			if (vertices.empty()) {
				for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
					vertices.emplace_back(device, 1);
					vertices.back().setPrimitiveType(static_cast<entity::PrimitiveType>(i));
				}
			}
			if (materialDatas.empty()) {
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					materialDatas.emplace_back(device);
					materialDatas.back().create(sizeof(Material), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			//std::cout<<"size : "<<gameObjects.size()<<std::endl;
			if (gameObjects.size() == 0) {
				/*std::cout<<"nb game objects : "<<gameObjects.size()<<std::endl;
				int i;
				std::cin>>i;*/
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputMeshes.emplace_back(device);
					outputMeshes.back().create(sizeof(entity::SubMesh), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < 1; i++) {
					inputMeshlets.emplace_back(device);
					inputMeshlets.back().create(sizeof(Meshlet), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputObjectDatas.emplace_back(device);
					outputObjectDatas.back().create(sizeof(ModelData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputModelDatas.emplace_back(device);
					outputModelDatas.back().create(sizeof(ModelData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputMaterialDatas.emplace_back(device);
					outputMaterialDatas.back().create(sizeof(Material), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			} else {
				unsigned int totalSubMeshes=0;
				for (unsigned int i = 0; i < gameObjects.size(); i++) {
					totalSubMeshes += gameObjects[i]->getGameObject()->getSubMeshes().size();
				}
				/*std::cout<<"reallocate output buffers "<<totalSubMeshes<<std::endl;
				int i;
				std::cin>>i;*/
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputMeshes.emplace_back(device);
					outputMeshes.back().create(sizeof(entity::SubMesh)*totalSubMeshes, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < 1; i++) {
					inputMeshlets.emplace_back(device);
					inputMeshlets.back().create(sizeof(Meshlet)*totalMeshlets, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputObjectDatas.emplace_back(device);
					outputObjectDatas.back().create(sizeof(ModelData)*totalSubMeshes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputModelDatas.emplace_back(device);
					outputModelDatas.back().create(sizeof(ModelData)*totalSubMeshes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
				for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
					outputMaterialDatas.emplace_back(device);
					outputMaterialDatas.back().create(sizeof(Material)*totalSubMeshes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				}
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				outputElementsDrawIndirectCommand.emplace_back(device);
				outputElementsDrawIndirectCommand.back().create(sizeof(DrawElementsIndirectCommand)*MAX_DRAW_INDIRECT_COMMANDS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				outputTaskDatas.emplace_back(device);
				outputTaskDatas.back().create(sizeof(TaskData)*MAX_TASKS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				drawCount.emplace_back(device);
				drawCount.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}			
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				taskCount.emplace_back(device);
				taskCount.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}			
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputMeshes.emplace_back(device);
				offsetInOutputMeshes.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputModelData.emplace_back(device);
				offsetInOutputModelData.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputObjectData.emplace_back(device);
				offsetInOutputObjectData.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputMaterialData.emplace_back(device);
				offsetInOutputMaterialData.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputElementsIndirectCommands.emplace_back(device);
				offsetInOutputElementsIndirectCommands.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				offsetInOutputTaskDatas.emplace_back(device);
				offsetInOutputTaskDatas.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT * NB_PRIMITIVE_TYPES; i++) {
				instanceBase.emplace_back(device);
				instanceBase.back().create(sizeof(unsigned int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				ubo.emplace_back(device);
				ubo.back().create(sizeof(UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
					VMA_ALLOCATION_CREATE_MAPPED_BIT);
			}
			setCamera(m_defaultCamera);
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				staggingVertexBufferDatas.emplace_back(device);
			}
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vertexBufferDatas.emplace_back(device);
				vertexBufferDatas.back().create(sizeof(VertexBufferData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			}			
			for (unsigned int i = 0; i < 1; i++) {
				for (unsigned int j = 0; j < NB_PRIMITIVE_TYPES; j++) {
					//std::cout<<"update vertices"<<std::endl;
					vertices[j].setPrimitiveType(static_cast<entity::PrimitiveType>(j));
					vertices[j].update( i);
				}
			}			
			VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
			depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilCreateInfo.depthTestEnable = VK_FALSE;
			depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
			depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
			depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional
			depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
			depthStencilInfos.emplace_back(depthStencilCreateInfo);			
			if (useDepthTest()) {
				//std::cout<<"create depth test pipeline infos"<<std::endl;
				depthStencilCreateInfo.depthTestEnable = VK_TRUE;
				depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
				depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
				depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
				depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional
				depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
				depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				depthStencilInfos.emplace_back(depthStencilCreateInfo);
			}
			if (useStencilTest()) {
				depthStencilCreateInfo.depthTestEnable = VK_FALSE;
				depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
				depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
				depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
				depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional
				depthStencilCreateInfo.stencilTestEnable = VK_TRUE;
				depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
				depthStencilInfos.emplace_back(depthStencilCreateInfo);
				depthStencilCreateInfo.depthTestEnable = VK_TRUE;
				depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
				depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
				depthStencilCreateInfo.minDepthBounds = 0.0f; // Optional
				depthStencilCreateInfo.maxDepthBounds = 1.0f; // Optional
				depthStencilCreateInfo.stencilTestEnable = VK_TRUE;
				depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				depthStencilInfos.emplace_back(depthStencilCreateInfo);
			}
			//std::cout<<"create descriptors and pipelines"<<std::endl;
			createDescriptorAndPipelines();
			createSynchPrimitives();
			needToUpdateDescriptorSets = true;
			needToUpdateBuffers = true;
			depthTestEnabled = stencilTestEnabled = false;
			//std::cout<<"initialized"<<std::endl;
		}
		void RenderTarget::createSynchPrimitives() {			
			/*computeFinishedSemaphores.reserve(MAX_FRAMES_IN_FLIGHT);
			computeFences.reserve(MAX_FRAMES_IN_FLIGHT);
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				computeFinishedSemaphores.emplace_back(device);
				computeFinishedSemaphores[i].create();
				computeFences.emplace_back(device);
				computeFences[i].create();
			}	*/
		}
		void RenderTarget::updateBuffers() {
			if (needToUpdateBuffers && gameObjects.size() > 0) {
				//std::cout<<"update buffers"<<std::endl;
				currentSubmeshesOffset = 0;
				currentModelDataOffset = 0;
				uint32_t totalMeshlets = 0;
				for (unsigned int i = 0; i < NB_PRIMITIVE_TYPES; i++) {
					vertices[i].clear();
					currentVertexOffset[i] = 0;
					currentIndexOffset[i] = 0;
				}
				Material::updateIds();
				std::deque<Material*> materials = Material::getAllMaterials();
				std::vector<MaterialData> materialDatas;
				//std::cout<<"material : "<<materials.size()<<std::endl;
				for (unsigned int i = 0; i < materials.size(); i++) {
					MaterialData material;
					//std::cout<<"material : "<<materials[i]<<std::endl;
					material.diffuseTextureIndex = (materials[i]->getTexture(entity::SubMesh::DIFFUSE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::DIFFUSE)->getId() : 0;
					material.specularTextureIndex = (materials[i]->getTexture(entity::SubMesh::SPECULAR) != nullptr) ? materials[i]->getTexture(entity::SubMesh::SPECULAR)->getId() : 0;
					material.normalTextureIndex = (materials[i]->getTexture(entity::SubMesh::NORMAL) != nullptr) ? materials[i]->getTexture(entity::SubMesh::NORMAL)->getId() : 0;
					material.metalnessTextureIndex = (materials[i]->getTexture(entity::SubMesh::METALNESS) != nullptr) ? materials[i]->getTexture(entity::SubMesh::METALNESS)->getId() : 0;
					material.roughnessTextureIndex = (materials[i]->getTexture(entity::SubMesh::ROUGHNESS) != nullptr) ? materials[i]->getTexture(entity::SubMesh::ROUGHNESS)->getId() : 0;
					material.aoTextureIndex = (materials[i]->getTexture(entity::SubMesh::AO) != nullptr) ? materials[i]->getTexture(entity::SubMesh::AO)->getId() : 0;
					material.emissiveTextureIndex = (materials[i]->getTexture(entity::SubMesh::EMISSIVE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::EMISSIVE)->getId() : 0;
					material.uvScale = /*(materials[i]->getTexture(Material::DIFFUSE) != nullptr) ? math::Vec2f(1.f / materials[i]->getTexture(Material::DIFFUSE)->getSize().x(), 1.f / materials[i]->getTexture(Material::DIFFUSE)->getSize().y()) :*/ math::Vec2f(1.f, 1.f);
					material.uvOffset = math::Vec2f(0.f, 0.f);
					material.materialType = materials[i]->getType();
					material.nbBuffers = (materials[i]->getTexture(entity::SubMesh::DIFFUSE) != nullptr) ? materials[i]->getTexture(entity::SubMesh::DIFFUSE)->getNbBuffers() : 0;
					material.vertsInstanceSet  = 0;
					material.nbVertices = 0;
					material.nbIndexes = 0;
					material.materialId = materials[i]->getId();
					material.instanceGroupId = materials[i]->getInstanceGroup();
					materialDatas.push_back(material);

				}
				std::vector<Object> objectDatas;
				std::vector<ModelData> modelDatas;
				std::vector<SubMeshData> subMeshesDatas;
				std::vector<LODLevelData> lodLevelDatas;
				std::vector<Meshlet> meshletDatas;
				for (unsigned int i = 0; i < gameObjects.size(); i++) {
					Object object;
					physic::BoundingBox globalBounds = gameObjects[i]->getGameObject()->getGlobalBounds();
					gameObjects[i]->getGameObject()->subMeshOffset = currentSubmeshesOffset;
					AABB aabb;
					aabb.center = globalBounds.getCenter();
					aabb.size = globalBounds.getSize();
					object.globalBounds = aabb;
					object.type = gameObjects[i]->getGameObject()->getTypeInt();
					object.subMeshesOffset = currentSubmeshesOffset;
					object.modelDataOffset = currentModelDataOffset;
					object.nbSubMeshes = gameObjects[i]->getGameObject()->getSubMeshesCount();
					objectDatas.push_back(object);
					ModelData modelData;
					modelData.modelMatrix = gameObjects[i]->getGameObject()->getTransform().getMatrix().transpose();
					//std::cout<<"model matrix : "<<gameObjects[i]->getTransform().getMatrix()<<std::endl;
					modelData.shadowProjMatrix = math::Matrix4f();
					modelData.borderMatrices = math::Matrix4f();
					modelDatas.push_back(modelData);
					currentModelDataOffset++;
					for (unsigned int j = 0; j < gameObjects[i]->getGameObject()->getSubMeshesCount(); j++) {
						//std::cout<<"add subMesh : "<<j<<std::endl;
						entity::SubMesh& subMesh = gameObjects[i]->getGameObject()->getSubMeshes()[j];						
						gameObjects[i]->getGameObject()->getSubMeshes()[j].verticesOffset = currentVertexOffset[subMesh.getVertexArray().getPrimitiveType()];
						SubMeshData subMeshData;
						physic::BoundingBox subMeshGlobalBounds = subMesh.getVertexArray().getGlobalBounds(gameObjects[i]->getGameObject()->getTransform());
						/*if (!subMeshGlobalBounds.intersects(m_camera.getViewVolume())) {
							std::cout<<"out of furstrum"<<std::endl;
						}*/
						AABB subMeshAABB;
						subMeshData.id = currentSubmeshesOffset;
						//std::cout<<"submesh offset : "<<currentSubmeshesOffset<<","<<gameObjects[i]->getSubMeshesCount()<<std::endl;
						subMeshAABB.center = subMeshGlobalBounds.getCenter();
						subMeshAABB.size = subMeshGlobalBounds.getSize();
						subMeshData.globalBounds = subMeshAABB;
						unsigned int primitiveType = subMesh.getVertexArray().getPrimitiveType();
						subMeshData.primitiveType = primitiveType;
						subMeshData.vertexOffset = currentVertexOffset[primitiveType];
						subMeshData.indexOffset = currentIndexOffset[primitiveType];
						subMeshData.materialId = gameObjects[i]->getMaterials()[j]->getId();
						subMeshData.nbVertices = subMesh.getVertexArray().getVertexCount();
						subMeshData.nbIndexes = subMesh.getVertexArray().getIndexCount();
						subMeshData.objectId = i;
						//std::cout<<"vertices count : "<<subMesh.getVertexBuffer().getVertexCount()<<std::endl;
						/*int pause;
						std::cin>>pause;*/
						//std::cout<<"index count : "<<subMesh.getVertexBuffer().getIndexCount()<<std::endl;

						/*std::cout<<"total vertex count : "<<vertices[primitiveType].getVertexCount()<<std::endl;
						std::cout<<"total index count : "<<vertices[primitiveType].getIndexCount()<<std::endl;*/
						//std::cout<<"vertx offset, texture id : "<<currentVertexOffset[primitiveType]<<","<<subMeshData.materialId<<std::endl;
						subMesh.getVertexArray().updateLods();
						//std::cout<<"vertex offset : "<<currentVertexOffset[primitiveType]<<std::endl;
						unsigned int baseVertex = currentVertexOffset[primitiveType];
						for (unsigned int v = 0; v < subMesh.getVertexArray().getVertexCount(); v++) {
							//std::cout<<"add vertex : "<<subMesh.getVertexBuffer()[v].position<<std::endl;
							vertices[primitiveType].append(subMesh.getVertexArray()[v]);
							vertices[primitiveType][baseVertex+v].drawableDataId = currentSubmeshesOffset;
							//std::cout<<"drawable data id : "<<vertices[primitiveType][v].drawableDataId<<std::endl;
							currentVertexOffset[primitiveType]++;
						}
						for (unsigned int v = 0; v < subMesh.getVertexArray().getIndexCount(); v++) {
							//std::cout<<"add index"<<std::endl;
							vertices[primitiveType].addIndex( subMesh.getVertexArray().getIndex(v));
							currentIndexOffset[primitiveType]++;
						}
						/*std::cout<<"new total vertex count : "<<vertices[primitiveType].getVertexCount()<<std::endl;
						std::cout<<"new total index count : "<<vertices[primitiveType].getIndexCount()<<std::endl;*/

						std::array<entity::VertexArray::LODLevel, 5> lods = subMesh.getVertexArray().getLODs();
						unsigned int currentSubmeshMeshletOffset = meshletDatas.size();
						
						for (unsigned int l = 0; l < lods.size(); l++) {
							LODLevelData lodLevelData{};
							lodLevelData.index_offset = lods[l].indexOffset;
							lodLevelData.index_count = lods[l].indexCount;							
							unsigned int currentLodMeshletOffset = meshletDatas.size() - currentSubmeshMeshletOffset;
							//std::cout<<"currentSubmeshMeshletOffset : "<<currentSubmeshMeshletOffset<<" "<<currentLodMeshletOffset<<","<<meshletDatas.size()<<std::endl;
							lodLevelData.meshletOffset = currentLodMeshletOffset;
							/*TriangleBatch triBatchInit;
							triBatchInit.minVertex = std::numeric_limits<unsigned int>::max();
							triBatchInit.maxVertex = 0;
							unsigned int nbBatchs = (lods[l].indexCount / 3)/MAX_PRIMS+1;
							std::vector<TriangleBatch> triBatchs(nbBatchs, triBatchInit);*/
							Meshlet m;
							m.minVertex = std::numeric_limits<unsigned int>::max();
							m.maxVertex = 0;
							m.nbIndexes = 0;
							m.indexOffset = 0;														
							for (unsigned int tri = 0; tri < lods[l].indexCount / 3; tri++) {
								int g0 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset+tri*3+0);								
								int g1 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset+tri*3+1);
								int g2 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset+tri*3+2);																
								unsigned int newMin = std::min(g0, std::min(g1, g2));
    							unsigned int newMax = std::max(g0, std::max(g1, g2));
								newMin = std::min(m.minVertex, newMin);
								newMax = std::max(m.maxVertex, newMax);
								unsigned int newVertexCount = (newMin - newMax) + 1;
								// Si ce triangle dépasse les limites → nouveau meshlet
								if (m.nbIndexes/3 >= MAX_PRIMS || newVertexCount > MAX_VERTS)
    							{
									m.vertexOffset = m.minVertex;
									m.nbVertices   = m.maxVertex - m.minVertex + 1;
									meshletDatas.push_back(m);
									// Nouveau meshlet
									m = Meshlet();
									m.minVertex = std::numeric_limits<unsigned int>::max();
									m.maxVertex = 0;
									m.nbIndexes = 0;
									m.indexOffset = tri * 3;
									// Recalculer pour ce triangle
									newMin = std::min(g0, std::min(g1, g2));
									newMax = std::max(g0, std::max(g1, g2));
									newVertexCount = newMax - newMin + 1;									
								}
								// Ajouter triangle
								m.minVertex = newMin;
								m.maxVertex = newMax;
								m.nbIndexes += 3;
							}
							// Final meshlet
							m.vertexOffset = m.minVertex;
							m.nbVertices   = m.maxVertex - m.minVertex + 1;
							meshletDatas.push_back(m);
							//std::cout<<"lod meshlet count : "<<(meshletDatas.size() - currentLodMeshletOffset - currentSubmeshMeshletOffset)<<std::endl;
							lodLevelData.meshletOffset = currentLodMeshletOffset;
							lodLevelData.meshletCount = meshletDatas.size() - currentLodMeshletOffset - currentSubmeshMeshletOffset;
							//std::cout<<"meshlet count : "<<	meshletDatas.size()<<","<<currentLodMeshletOffset<<std::endl;																		
							lodLevelDatas.push_back(lodLevelData);
							for (unsigned int m = currentSubmeshMeshletOffset+lodLevelData.meshletOffset; m < currentSubmeshMeshletOffset+lodLevelData.meshletOffset + lodLevelData.meshletCount; m++) {
								Meshlet meshlet = meshletDatas[m];
								unsigned int firstIndex = subMeshData.indexOffset + lods[l].indexOffset + meshlet.indexOffset; 
								//std::cout<<"meshlet : "<<lodLevelData.meshletCount<<","<<currentSubmeshMeshletOffset<<","<<subMeshData.indexOffset<<","<<meshlet.nbIndexes<<","<<meshlet.nbVertices<<","<<meshlet.indexOffset<<","<<meshlet.vertexOffset<<std::endl;							
								for (unsigned int t = 0; t < meshlet.nbIndexes  / 3; t++) {
									int i0 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset + meshlet.indexOffset+t*3+0) - meshlet.vertexOffset;
									int i1 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset + meshlet.indexOffset+t*3+1) - meshlet.vertexOffset;
									int i2 = vertices[primitiveType].getIndex(subMeshData.indexOffset + lods[l].indexOffset + meshlet.indexOffset+t*3+2) - meshlet.vertexOffset;
									if (i0 >= meshlet.nbVertices || i1 >= meshlet.nbVertices || i2 >= meshlet.nbVertices) {
										std::cout<<"indexes : "<<i0<<","<<i1<<","<<i2<<","<<meshlet.nbVertices<<std::endl;
										system("PAUSE");
									}
								}
							}
						}											
						subMeshData.meshletOffset = currentSubmeshMeshletOffset;
						subMeshData.meshletCount = meshletDatas.size() - currentSubmeshMeshletOffset;
						subMeshData.lodOffset = currentSubmeshesOffset * lods.size();
						//std::cout<<"submesh meshlet count : "<<(meshletDatas.size() - currentSubmeshMeshletOffset)<<std::endl;
						subMeshesDatas.push_back(subMeshData);	
						currentSubmeshesOffset++;					
					}									
				}
				totalMeshlets = meshletDatas.size();
				totalSubMeshes = subMeshesDatas.size();
				/*std::cout<<"total sub meshes"<<totalSubMeshes<<std::endl;
				system("PAUSE");*/
				for (unsigned int i = 0; i < 1; i++) {
					for (unsigned int j = 0; j < NB_PRIMITIVE_TYPES; j++) {
						//std::cout<<"update vertices"<<std::endl;
						vertices[j].setPrimitiveType(static_cast<entity::PrimitiveType>(j));
						vertices[j].update(commandPool.getHandle(i), i);
					}
				}
				//std::cout<<"max lod index : "<<subMeshesDatas.size() * 5<<std::endl;
				for (unsigned int i = 0; i < 1; i++) {
					//std::cout<<"copy objects"<<std::endl;
					staggingObjects[i].create(objectDatas.size() * sizeof(Object), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingObjects[i].update(objectDatas.data(), objectDatas.size() * sizeof(Object));
					objects[i].create(objectDatas.size() * sizeof(Object), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
					Buffer::copyBuffer(staggingObjects[i], objects[i], objectDatas.size() * sizeof(Object), commandPool.getHandle(getCurrentFrame()));
					//std::cout<<"stagging object buffer : "<<staggingObjects[i].getHandle()<<std::endl;
					//std::cout<<"object buffer : "<<staggingObjects[i].getHandle()<<std::endl;
				}
				for (unsigned int i = 0; i < 1; i++) {
					//std::cout<<"copy submeshes"<<std::endl;
					staggingSubMeshes[i].create(subMeshesDatas.size() * sizeof(SubMeshData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingSubMeshes[i].update(subMeshesDatas.data(), subMeshesDatas.size() * sizeof(SubMeshData));
					subMeshes[i].create(subMeshesDatas.size() * sizeof(SubMeshData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
					Buffer::copyBuffer(staggingSubMeshes[i], subMeshes[i], subMeshesDatas.size() * sizeof(SubMeshData), commandPool.getHandle(getCurrentFrame()));
					//std::cout<<"stagging submeshes buffer : "<<staggingObjects[i].getHandle()<<std::endl;
					//std::cout<<"submeshes buffer : "<<staggingObjects[i].getHandle()<<std::endl;
				}
				for (unsigned int i = 0; i < 1; i++) {
					staggingMeshlets[i].create(meshletDatas.size() * sizeof(Meshlet), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingMeshlets[i].update(meshletDatas.data(), meshletDatas.size() * sizeof(Meshlet));
					inputMeshlets[i].create(meshletDatas.size() * sizeof(Meshlet), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
					Buffer::copyBuffer(staggingMeshlets[i], inputMeshlets[i], meshletDatas.size() * sizeof(Meshlet), commandPool.getHandle(getCurrentFrame()));
				}
				for (unsigned int i = 0; i < 1; i++) {
					//std::cout<<"copy submeshes"<<std::endl;
					staggingLODLevel[i].create(lodLevelDatas.size() * sizeof(LODLevelData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingLODLevel[i].update(lodLevelDatas.data(), lodLevelDatas.size() * sizeof(LODLevelData));
					lodLevel[i].create(lodLevelDatas.size() * sizeof(LODLevelData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
					Buffer::copyBuffer(staggingLODLevel[i], lodLevel[i], lodLevelDatas.size() * sizeof(LODLevelData), commandPool.getHandle(getCurrentFrame()));
					//std::cout<<"stagging submeshes buffer : "<<staggingObjects[i].getHandle()<<std::endl;
					//std::cout<<"submeshes buffer : "<<staggingObjects[i].getHandle()<<std::endl;
				}
				for (unsigned int i = 0; i < 1; i++) {
					//std::cout<<"copy model datas : "<<modelDatas.size()<<std::endl;
					staggingModelDatas[i].create(modelDatas.size() * sizeof(ModelData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingModelDatas[i].update(modelDatas.data(), modelDatas.size() * sizeof(ModelData));
					this->modelDatas[i].create(modelDatas.size() * sizeof(ModelData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
					Buffer::copyBuffer(staggingModelDatas[i], this->modelDatas[i], modelDatas.size() * sizeof(ModelData), commandPool.getHandle(getCurrentFrame()));
					//std::cout<<"stagging object buffer : "<<staggingObjects[i].getHandle()<<std::endl;
				}
				for (unsigned int i = 0; i < 1; i++) {
					staggingMaterialDatas[i].create(materialDatas.size() * sizeof(MaterialData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
					staggingMaterialDatas[i].update(materialDatas.data(), materialDatas.size() * sizeof(MaterialData));
					for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
						for (unsigned int k = 0; k < NB_PRIMITIVE_TYPES; k++) {
							this->materialDatas[k * MAX_FRAMES_IN_FLIGHT + j].create(materialDatas.size() * sizeof(MaterialData), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
							Buffer::copyBuffer(staggingMaterialDatas[i], this->materialDatas[k * MAX_FRAMES_IN_FLIGHT + j], materialDatas.size() * sizeof(MaterialData), commandPool.getHandle(getCurrentFrame()));
						}
					}
				}
				
				for (unsigned int j = 0; j < MAX_FRAMES_IN_FLIGHT; j++) {
					for (unsigned int k = 0; k < NB_PRIMITIVE_TYPES; k++) {
						//std::cout<<"update : "<<j<<","<<k<<std::endl;
						outputObjectDatas[j * NB_PRIMITIVE_TYPES + k].create(sizeof(ModelData) * currentSubmeshesOffset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
						outputModelDatas[j * NB_PRIMITIVE_TYPES + k].create(sizeof(ModelData) * currentSubmeshesOffset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
						//std::cout<<"output model data : "<<outputModelDatas[j * NB_PRIMITIVE_TYPES + k].getRange()<<std::endl;
						outputMaterialDatas[j * NB_PRIMITIVE_TYPES + k].create(sizeof(MaterialData) * currentSubmeshesOffset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
						outputMeshes[j * NB_PRIMITIVE_TYPES + k].create(sizeof(entity::SubMesh) * currentSubmeshesOffset, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);						
						//std::cout<<"output material datas : "<<registeredRenderTargets[i]->outputMaterialDatas[j * NB_PRIMITIVE_TYPES + k].getRange()<<std::endl;
						//std::cout<<"output material datas : "<<registeredRenderTargets[i]->outputMaterialDatas[j * NB_PRIMITIVE_TYPES + k].getRange()<<std::endl;
					}
				}
				
				//std::cout<<"buffers updated"<<std::endl;
				needToUpdateBuffers = false;
				commandPool.endRecordCommandBuffer(getCurrentFrame());
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &commandPool.getHandle(getCurrentFrame());
				Device::QueueFamilyIndices indices = GPUContext::instance().getDevice().findQueueFamilies(GPUContext::instance().getDevice().getPhysicalDevice());
				
				std::unique_lock lock(mtx);	
					//std::cout<<"wait!"<<std::endl;				
				cv.wait(lock, [this]{return ParticleSystemUpdater::instance(cv, mtx).isSubmitReady() && MorphAnimUpdater::instance(cv, mtx).isSubmitReady() && BoneAnimUpdater::instance(cv, mtx).isSubmitReady();});
				
				//std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
				if (vkQueueSubmit(GPUContext::instance().getDevice().getQueue(indices.graphicsFamily.value(), 0), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
					throw std::runtime_error("Echec de l'envoi d'un command buffer!");
				}
				vkDeviceWaitIdle(device.getDevice());
				commandPool.beginRecordCommandBuffer(getCurrentFrame());
				//vertices.clear();
				needToUpdateDescriptorSets = true;

				//std::cout<<"data updated!"<<std::endl;
				ParticleSystemUpdater::instance(cv, mtx).setBuffersReady(true);
				ParticleSystemUpdater::instance(cv, mtx).cv3.notify_all();

				MorphAnimUpdater::instance(cv, mtx).setBuffersReady(true);
				MorphAnimUpdater::instance(cv, mtx).cv3.notify_all();
				
				BoneAnimUpdater::instance(cv, mtx).setBuffersReady(true);
				BoneAnimUpdater::instance(cv, mtx).cv3.notify_all();
				
			}	
		}
		void RenderTarget::setTypesToRender(std::string expression, unsigned int currentFrame) {
			std::vector<int> objectTypes;
			for (unsigned int i = 0; i < entity::Entity::getNbEntitiesTypes(); i++) {
				//std::cout<<"add type : "<<i<<std::endl;
				objectTypes.push_back(i);
			}
			cullingInfo.nbEntitiesTypes = entity::Entity::getNbEntitiesTypes();
				//std::cout<<"ubo datas : "<<&cullingInfo<<","<<sizeof(UBO)<<std::endl;
			ubo[currentFrame].update(&cullingInfo, sizeof(UBO));
			staggingObjectTypes[currentFrame].create(objectTypes.size() * (sizeof(int)), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
			staggingObjectTypes[currentFrame].update(objectTypes.data(), sizeof(int) * objectTypes.size());
			this->objectTypes[currentFrame].create(sizeof(int) * objectTypes.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			Buffer::copyBuffer(staggingObjectTypes[currentFrame], this->objectTypes[currentFrame], sizeof(int) * objectTypes.size(), commandPool.getHandle(getCurrentFrame()));
			needToUpdateDescriptorSets = true;
			needToUpdateCullBatchIndCmds = true;
		}
		std::vector<VkPipelineDepthStencilStateCreateInfo> RenderTarget::getDepthStencilInfos() {
			return depthStencilInfos;
		}
		void RenderTarget::createDescriptorAndPipelines() {
			if (device.areMeshShadersSupported()) {
				std::vector<VkPipelineRenderingCreateInfo> renderingCreateInfos = {};		
				VkPipelineRenderingCreateInfo renderingCreateInfo = {};
				renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
				renderingCreateInfo.colorAttachmentCount = (isDepthOnly()) ? 0 : 1;
				renderingCreateInfo.pColorAttachmentFormats = (isDepthOnly()) ? nullptr : &getImageFormat();
				renderingCreateInfo.depthAttachmentFormat = getDepthStencilTexture().getFormat();
				renderingCreateInfos.emplace_back(renderingCreateInfo);			
				DescriptorSetLayout& resetBuffersLayout = GPUContext::instance().getDescriptorSetLayout(resetBuffersShader, 7, false);
				for (unsigned int i = 0; i < 7; i++) {
					resetBuffersLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				}			
				resetBuffersLayout.update();
				std::vector<VkPushConstantRange> pushConstants;
				VkPushConstantRange pushConstant;
				pushConstant.offset = 0;
				pushConstant.size = sizeof(CullingBatchingPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
				pushConstants.push_back(pushConstant);
				//std::cout<<"reset compute pipeline"<<std::endl;
				GPUContext::instance().getComputePipeline(resetBuffersShader).createComputePipeline(resetBuffersShader, GPUContext::instance().getDescriptorSetLayout(resetBuffersShader), pushConstants);
				DescriptorSetLayout& cullingBatchingLayout = GPUContext::instance().getDescriptorSetLayout(cullingBatchingShader, 17, false);
				cullingBatchingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				for (unsigned int i = 1; i < 4; i++) {
					cullingBatchingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
				}
				for (unsigned int i = 4; i < 15; i++) {
					cullingBatchingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				}			
				cullingBatchingLayout.updateLayout(15, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				cullingBatchingLayout.updateLayout(16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
				cullingBatchingLayout.update();
				//std::cout<<"create culling compute pipeline"<<std::endl;
				GPUContext::instance().getComputePipeline(cullingBatchingShader).createComputePipeline(cullingBatchingShader, GPUContext::instance().getDescriptorSetLayout(cullingBatchingShader), pushConstants);
				BlendMode blendMode;				
				pushConstants.clear();
				pushConstant.offset = 0;
				pushConstant.size = sizeof(ViewProjMatPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT |VK_SHADER_STAGE_MESH_BIT_EXT;
				pushConstants.push_back(pushConstant);
				VkPushConstantRange frag_push_constant;
				frag_push_constant.offset = sizeof(ViewProjMatPC);
				frag_push_constant.size = sizeof(IndexesPC);
				frag_push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstants.push_back(frag_push_constant);
				DescriptorSetLayout& defaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 10, true);				
				for (unsigned int i = 0; i < 3; i++) {
					defaultRenderingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT);
				}
				defaultRenderingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_MESH_BIT_EXT);				
				for (unsigned int i = 4; i < 6; i++) {
					defaultRenderingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES, VK_SHADER_STAGE_MESH_BIT_EXT);
				}
				for (unsigned int i = 6; i < 8; i++) {
					defaultRenderingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_MESH_BIT_EXT);
				}				
				defaultRenderingLayout.updateLayout(8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);					
				defaultRenderingLayout.updateLayout(9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				defaultRenderingLayout.update();
				DescriptorSetLayout& specularDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 1);
				specularDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				specularDefaultRenderingLayout.update();
				DescriptorSetLayout& normalDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 2);
				normalDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				normalDefaultRenderingLayout.update();
				DescriptorSetLayout& metalnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 3);
				metalnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				metalnessDefaultRenderingLayout.update();
				DescriptorSetLayout& roughnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 4);
				roughnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				roughnessDefaultRenderingLayout.update();
				DescriptorSetLayout& aoDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 5);
				aoDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				aoDefaultRenderingLayout.update();
				DescriptorSetLayout& emissiveDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 6);
				emissiveDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				emissiveDefaultRenderingLayout.update();
				if (!isDepthOnly()) {
					for (unsigned int i = 0; i < depthStencilInfos.size(); i++) {
						/*blendMode = BlendAlpha;
						blendMode.updateIds();
						std::cout<<"pipeline  : "<<Triangles<<","<<defaultRenderingShader.getId()<<","<<blendMode.id<<","<<i<<std::endl;*/
						GPUContext::instance().getGraphicsPipeline(entity::PrimitiveType::Triangles, defaultRenderingShader, blendMode,i).createGraphicPipeline( defaultRenderingShader, GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader), renderingCreateInfos.back(), depthStencilInfos[i], blendMode, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
					}
				}
				//std::cout<<"descriptor and pipelines created default render target"<<std::endl;
				DescriptorSetLayout& vertexBufferLayout = GPUContext::instance().getDescriptorSetLayout(vertexBufferShader, 1, true);				
				vertexBufferLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				pushConstants.clear();
				pushConstant.offset = 0;
				pushConstant.size = sizeof(VertexBufferPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				pushConstants.push_back(pushConstant);
				vertexBufferLayout.update();
				if (!isDepthOnly()) {
					for (unsigned int i = 0; i < depthStencilInfos.size(); i++) {
						for (unsigned int p = 0; p < NB_PRIMITIVE_TYPES; p++) {
							GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(p), vertexBufferShader, blendMode,i).createGraphicPipeline( vertexBufferShader, static_cast<entity::PrimitiveType>(p),GPUContext::instance().getDescriptorSetLayout(vertexBufferShader), renderingCreateInfos.back(), depthStencilInfos[i], blendMode, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
						}
					}
				}
				//std::cout<<"descriptor and pipelines created vertex buffer"<<std::endl;

				DescriptorPool& resetBuffersPool = GPUContext::instance().getDescriptorPool(resetBuffersShader, 7);
				for (unsigned int i = 0; i < 7; i++) {
					resetBuffersPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				resetBuffersPool.update();
				DescriptorPool& cullingBatchingPool = GPUContext::instance().getDescriptorPool(cullingBatchingShader, 17);
				cullingBatchingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
				for (unsigned int i = 1; i < 4; i++) {
					cullingBatchingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
				}
				for (unsigned int i = 4; i < 15; i++) {
					cullingBatchingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				cullingBatchingPool.updatePoolSize(15, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
				cullingBatchingPool.updatePoolSize(16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
				cullingBatchingPool.update();
				DescriptorPool& defaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 10);
				for (unsigned int i = 0; i < 4; i++) {
					defaultRenderingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				for (unsigned int i = 4; i < 6; i++) {
					defaultRenderingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES);
				}
				for (unsigned int i = 6; i < 8; i++) {
					defaultRenderingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
				}
				defaultRenderingPool.updatePoolSize(8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				defaultRenderingPool.updatePoolSize(9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				defaultRenderingPool.update();
				DescriptorPool& specularDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 1);
				specularDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				specularDefaultRenderingPool.update();
				DescriptorPool& normalDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 2);
				normalDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				normalDefaultRenderingPool.update();
				DescriptorPool& metalnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 3);
				metalnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				metalnessDefaultRenderingPool.update();
				DescriptorPool& roughnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 4);
				roughnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				roughnessDefaultRenderingPool.update();
				DescriptorPool& aoDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 5);
				aoDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				aoDefaultRenderingPool.update();
				DescriptorPool& emissiveDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 6);
				emissiveDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				emissiveDefaultRenderingPool.update();
				DescriptorPool& vertexBufferPool = GPUContext::instance().getDescriptorPool(vertexBufferShader, 1);				
				vertexBufferPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				vertexBufferPool.update();
				//std::cout<<"descriptor pool : "<<resetBuffersPool.getHandle()<<std::endl;
				DescriptorSet::allocate(resetBuffersPool, resetBuffersLayout, GPUContext::instance().getDescriptorSets(resetBuffersShader, 7, 1));
				//std::cout<<"descriptor pool : "<<cullingBatchingPool.getHandle()<<std::endl;
				DescriptorSet::allocate(cullingBatchingPool, cullingBatchingLayout, GPUContext::instance().getDescriptorSets(cullingBatchingShader, 17, 1));
				//std::cout<<"descriptor pool : "<<defaultRenderingPool.getHandle()<<std::endl;
				DescriptorSet::allocate(defaultRenderingPool, defaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 10, 1), MAX_TEXTURES);
				DescriptorSet::allocate(specularDefaultRenderingPool, specularDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 1), MAX_TEXTURES);
				DescriptorSet::allocate(normalDefaultRenderingPool, normalDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 2), MAX_TEXTURES);
				DescriptorSet::allocate(metalnessDefaultRenderingPool, metalnessDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 3), MAX_TEXTURES);
				DescriptorSet::allocate(roughnessDefaultRenderingPool, roughnessDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 4), MAX_TEXTURES);
				DescriptorSet::allocate(aoDefaultRenderingPool, aoDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 5), MAX_TEXTURES);
				DescriptorSet::allocate(emissiveDefaultRenderingPool, emissiveDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 6), MAX_TEXTURES);
				//std::cout<<"descriptor pool : "<<vertexBufferPool.getHandle()<<std::endl;
				DescriptorSet::allocate(vertexBufferPool, vertexBufferLayout, GPUContext::instance().getDescriptorSets(vertexBufferShader, 1,1), MAX_TEXTURES);
				//std::cout<<"descriptor set vertex buffer allocated"<<std::endl;
			} else {
				std::vector<VkPipelineRenderingCreateInfo> renderingCreateInfos = {};		
				VkPipelineRenderingCreateInfo renderingCreateInfo = {};
				renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
				renderingCreateInfo.colorAttachmentCount = (isDepthOnly()) ? 0 : 1;
				renderingCreateInfo.pColorAttachmentFormats = (isDepthOnly()) ? nullptr : &getImageFormat();
				renderingCreateInfo.depthAttachmentFormat = getDepthStencilTexture().getFormat();
				renderingCreateInfos.emplace_back(renderingCreateInfo);			
				DescriptorSetLayout& resetBuffersLayout = GPUContext::instance().getDescriptorSetLayout(resetBuffersShader, 7, false);
				for (unsigned int i = 0; i < 7; i++) {
					resetBuffersLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				}			
				resetBuffersLayout.update();
				std::vector<VkPushConstantRange> pushConstants;
				VkPushConstantRange pushConstant;
				pushConstant.offset = 0;
				pushConstant.size = sizeof(CullingBatchingPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
				pushConstants.push_back(pushConstant);
				//std::cout<<"reset compute pipeline"<<std::endl;
				GPUContext::instance().getComputePipeline(resetBuffersShader).createComputePipeline(resetBuffersShader, GPUContext::instance().getDescriptorSetLayout(resetBuffersShader), pushConstants);
				DescriptorSetLayout& cullingBatchingLayout = GPUContext::instance().getDescriptorSetLayout(cullingBatchingShader, 17, false);
				cullingBatchingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				for (unsigned int i = 1; i < 4; i++) {
					cullingBatchingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
				}
				for (unsigned int i = 4; i < 15; i++) {
					cullingBatchingLayout.updateLayout(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				}			
				cullingBatchingLayout.updateLayout(15, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_COMPUTE_BIT);
				cullingBatchingLayout.updateLayout(16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT);
				cullingBatchingLayout.update();
				//std::cout<<"create culling compute pipeline"<<std::endl;
				GPUContext::instance().getComputePipeline(cullingBatchingShader).createComputePipeline(cullingBatchingShader, GPUContext::instance().getDescriptorSetLayout(cullingBatchingShader), pushConstants);
				BlendMode blendMode;
				pushConstants.clear();
				pushConstant.offset = 0;
				pushConstant.size = sizeof(ViewProjMatPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				pushConstants.push_back(pushConstant);
				VkPushConstantRange frag_push_constant;
				frag_push_constant.offset = sizeof(ViewProjMatPC);
				frag_push_constant.size = sizeof(IndexesPC);
				frag_push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				pushConstants.push_back(frag_push_constant);
				DescriptorSetLayout& defaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 4, true);
				defaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
				defaultRenderingLayout.updateLayout(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT);
				defaultRenderingLayout.updateLayout(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES*MAX_FRAMES_IN_FLIGHT, VK_SHADER_STAGE_FRAGMENT_BIT);
				defaultRenderingLayout.updateLayout(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				defaultRenderingLayout.update();
				DescriptorSetLayout& specularDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 1);
				specularDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				specularDefaultRenderingLayout.update();
				DescriptorSetLayout& normalDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 2);
				normalDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				normalDefaultRenderingLayout.update();
				DescriptorSetLayout& metalnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 3);
				metalnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				metalnessDefaultRenderingLayout.update();
				DescriptorSetLayout& roughnessDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 4);
				roughnessDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				roughnessDefaultRenderingLayout.update();
				DescriptorSetLayout& aoDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 5);
				aoDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				aoDefaultRenderingLayout.update();
				DescriptorSetLayout& emissiveDefaultRenderingLayout = GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader, 1, true, 6);
				emissiveDefaultRenderingLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				emissiveDefaultRenderingLayout.update();
				if (!isDepthOnly()) {
					for (unsigned int i = 0; i < depthStencilInfos.size(); i++) {
						for (unsigned int p = 0; p < NB_PRIMITIVE_TYPES; p++) {
							GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(p), defaultRenderingShader, blendMode,i).createGraphicPipeline( defaultRenderingShader, static_cast<entity::PrimitiveType>(p),GPUContext::instance().getDescriptorSetLayout(defaultRenderingShader), renderingCreateInfos.back(), depthStencilInfos[i], blendMode, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
							//std::cout<<"pipeline created in render target : "<<GPUContext::instance().getGraphicsPipeline(static_cast<PrimitiveType>(p), defaultRenderingShader, blendMode,i).getHandle()<<std::endl;
						}
					}
				}
				//std::cout<<"descriptor and pipelines created default render target"<<std::endl;
				DescriptorSetLayout& vertexBufferLayout = GPUContext::instance().getDescriptorSetLayout(vertexBufferShader, 1, true);
				vertexBufferLayout.updateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES/**MAX_FRAMES_IN_FLIGHT*/, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
					VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
				vertexBufferLayout.update();
				pushConstants.clear();
				pushConstant.offset = 0;
				pushConstant.size = sizeof(VertexBufferPC);
				pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				pushConstants.push_back(pushConstant);
				vertexBufferLayout.update();
				if (!isDepthOnly()) {
					for (unsigned int i = 0; i < depthStencilInfos.size(); i++) {
						for (unsigned int p = 0; p < NB_PRIMITIVE_TYPES; p++) {
							GPUContext::instance().getGraphicsPipeline(static_cast<entity::PrimitiveType>(p), vertexBufferShader, blendMode,i).createGraphicPipeline( vertexBufferShader, static_cast<entity::PrimitiveType>(p),GPUContext::instance().getDescriptorSetLayout(vertexBufferShader), renderingCreateInfos.back(), depthStencilInfos[i], blendMode, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL, pushConstants);
						}
					}
				}
				//std::cout<<"descriptor and pipelines created vertex buffer"<<std::endl;

				DescriptorPool& resetBuffersPool = GPUContext::instance().getDescriptorPool(resetBuffersShader, 7);
				for (unsigned int i = 0; i < 7; i++) {
					resetBuffersPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				resetBuffersPool.update();
				DescriptorPool& cullingBatchingPool = GPUContext::instance().getDescriptorPool(cullingBatchingShader, 17);
				cullingBatchingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT);
				for (unsigned int i = 1; i < 4; i++) {
					cullingBatchingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
				}
				for (unsigned int i = 4; i < 15; i++) {
					cullingBatchingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				cullingBatchingPool.updatePoolSize(15, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT);
				cullingBatchingPool.updatePoolSize(16, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
				cullingBatchingPool.update();
				DescriptorPool& defaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 4);
				for (unsigned int i = 0; i < 3; i++) {
					defaultRenderingPool.updatePoolSize(i, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, NB_PRIMITIVE_TYPES * MAX_FRAMES_IN_FLIGHT);
				}
				defaultRenderingPool.updatePoolSize(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				defaultRenderingPool.update();
				DescriptorPool& specularDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 1);
				specularDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				specularDefaultRenderingPool.update();
				DescriptorPool& normalDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 2);
				normalDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				normalDefaultRenderingPool.update();
				DescriptorPool& metalnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 3);
				metalnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				metalnessDefaultRenderingPool.update();
				DescriptorPool& roughnessDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 4);
				roughnessDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				roughnessDefaultRenderingPool.update();
				DescriptorPool& aoDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 5);
				aoDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				aoDefaultRenderingPool.update();
				DescriptorPool& emissiveDefaultRenderingPool = GPUContext::instance().getDescriptorPool(defaultRenderingShader, 1, 6);
				emissiveDefaultRenderingPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				emissiveDefaultRenderingPool.update();
				DescriptorPool& vertexBufferPool = GPUContext::instance().getDescriptorPool(vertexBufferShader, 1);
				vertexBufferPool.updatePoolSize(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES);
				vertexBufferPool.update();
				//std::cout<<"descriptor pool : "<<resetBuffersPool.getHandle()<<std::endl;
				DescriptorSet::allocate(resetBuffersPool, resetBuffersLayout, GPUContext::instance().getDescriptorSets(resetBuffersShader, 7, 1));
				//std::cout<<"descriptor pool : "<<cullingBatchingPool.getHandle()<<std::endl;
				DescriptorSet::allocate(cullingBatchingPool, cullingBatchingLayout, GPUContext::instance().getDescriptorSets(cullingBatchingShader, 17, 1));
				//std::cout<<"descriptor pool : "<<defaultRenderingPool.getHandle()<<std::endl;
				DescriptorSet::allocate(defaultRenderingPool, defaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 4, 1), MAX_TEXTURES);
				DescriptorSet::allocate(specularDefaultRenderingPool, specularDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 1), MAX_TEXTURES);
				DescriptorSet::allocate(normalDefaultRenderingPool, normalDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 2), MAX_TEXTURES);
				DescriptorSet::allocate(metalnessDefaultRenderingPool, metalnessDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 3), MAX_TEXTURES);
				DescriptorSet::allocate(roughnessDefaultRenderingPool, roughnessDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 4), MAX_TEXTURES);
				DescriptorSet::allocate(aoDefaultRenderingPool, aoDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 5), MAX_TEXTURES);
				DescriptorSet::allocate(emissiveDefaultRenderingPool, emissiveDefaultRenderingLayout, GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 6), MAX_TEXTURES);
				//std::cout<<"descriptor pool : "<<vertexBufferPool.getHandle()<<std::endl;
				DescriptorSet::allocate(vertexBufferPool, vertexBufferLayout, GPUContext::instance().getDescriptorSets(vertexBufferShader, 1,1), MAX_TEXTURES);
				//std::cout<<"descriptor set vertex buffer allocated"<<std::endl;
			}
		}
		void RenderTarget::updateDescriporSets() {
			
			if (device.areMeshShadersSupported()) {
				DescriptorSet& resetBuffersSet = GPUContext::instance().getDescriptorSets(resetBuffersShader, 7, 1)[0];
				//std::cout<<"update offset in output model data"<<std::endl;
				resetBuffersSet.updateBufferInfos(0, offsetInOutputModelData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output object data"<<std::endl;
				resetBuffersSet.updateBufferInfos(1, offsetInOutputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output material data"<<std::endl;
				resetBuffersSet.updateBufferInfos(2, offsetInOutputMaterialData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output indirect draw commands"<<std::endl;
				resetBuffersSet.updateBufferInfos(3, offsetInOutputTaskDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update input material datax"<<std::endl;
				resetBuffersSet.updateBufferInfos(4, materialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update first index vertex"<<std::endl;
				resetBuffersSet.updateBufferInfos(5, taskCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				resetBuffersSet.updateBufferInfos(6, instanceBase, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);				
				//std::cout<<"update reset buffer set"<<std::endl;
				resetBuffersSet.updateDescriptorSet();
				DescriptorSet& cullingBatchingSet = GPUContext::instance().getDescriptorSets(cullingBatchingShader, 17, 1)[0];
				//std::cout<<"update object types"<<std::endl;
				cullingBatchingSet.updateBufferInfos(0, objectTypes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update objects"<<std::endl;
				cullingBatchingSet.updateBufferInfos(1, objects, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range objects : "<<objects[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(2, subMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range  submeshes : "<<subMeshes[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(3, modelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range model datas : "<<modelDatas[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(4, materialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range material datas  : "<<materialDatas[0].getRange()<<std::endl;
				//std::cout<<"update output vertics"<<std::endl;
				cullingBatchingSet.updateBufferInfos(5, outputMaterialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output vertices : "<<outputVertexDatas[3].getVertexBuffer(0).getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(6, outputModelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output indexes : "<<outputVertexDatas[3].getIndexBuffer(0).getRange()<<std::endl;
				//std::cout<<"model data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(7, outputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output model datas : "<<outputModelDatas[0].getRange()<<std::endl;
				//std::cout<<"object data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(8, offsetInOutputModelData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output objects datas : "<<outputObjectDatas[0].getRange()<<std::endl;
				//std::cout<<"offset in output vertex"<<std::endl;
				cullingBatchingSet.updateBufferInfos(9, offsetInOutputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in output index"<<std::endl;
				cullingBatchingSet.updateBufferInfos(10, taskCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in model data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(11, outputTaskDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in output object data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(12, offsetInOutputTaskDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"uniform buffer"<<std::endl;
				cullingBatchingSet.updateBufferInfos(13, offsetInOutputMaterialData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				cullingBatchingSet.updateBufferInfos(14, instanceBase, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				cullingBatchingSet.updateBufferInfos(15, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				cullingBatchingSet.updateBufferInfos(16, lodLevel, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update culling ds"<<std::endl;
				cullingBatchingSet.updateDescriptorSet();
				bool hasDiffuseTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
				DescriptorSet& defaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, (hasDiffuseTextures) ? 10 : 9, 1)[0];
				defaultRenderingSet.updateBufferInfos(0, outputTaskDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(1, taskCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(2, outputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(3, outputModelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(4, true, vertices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(5, false, vertices, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(6, lodLevel, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(7, inputMeshlets, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);				
				defaultRenderingSet.updateBufferInfos(8, materialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				if (hasDiffuseTextures) {
					//std::cout<<"textures : "<<Texture::getAllTextures().size()<<std::endl;
					defaultRenderingSet.updateImageInfos(9, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				}
				defaultRenderingSet.updateDescriptorSet();
				bool hasSpecularTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
				if (hasSpecularTextures) {
					DescriptorSet& specularDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 1)[0];
					specularDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					specularDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasNormalTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
				if (hasNormalTextures) {
					DescriptorSet& normalDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 2)[0];
					normalDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					normalDefaultRenderingSet.updateDescriptorSet();
				}

				bool hasMetalnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
				if (hasMetalnessTextures) {
					DescriptorSet& metalnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 3)[0];
					metalnessDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					metalnessDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasRoughnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
				if (hasRoughnessTextures) {
					DescriptorSet& roughnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 4)[0];
					roughnessDefaultRenderingSet .updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					roughnessDefaultRenderingSet .updateDescriptorSet();
				}
				bool hasAOTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
				if (hasAOTextures) {
					DescriptorSet& aoDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 5)[0];
					aoDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					aoDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasEmissiveTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
				if (hasEmissiveTextures) {
					DescriptorSet& emissiveDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 6)[0];
					emissiveDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					emissiveDefaultRenderingSet.updateDescriptorSet();
				}
				//std::cout<<"update descriptor sets : "<<vertexBufferDatas[0].getRange()<<","<<vertexBufferDatas[1].getRange()<<std::endl;
				
				
				if (hasDiffuseTextures) {
					DescriptorSet& vertexBufferSet = GPUContext::instance().getDescriptorSets(vertexBufferShader, 1, 1)[0];
					vertexBufferSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(0), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					vertexBufferSet.updateDescriptorSet();
				}				
				/*std::cout<<"update descriptor sets"<<std::endl;
				system("PAUSE");*/
			} else {
				DescriptorSet& resetBuffersSet = GPUContext::instance().getDescriptorSets(resetBuffersShader, 7, 1)[0];
				//std::cout<<"update offset in output model data"<<std::endl;
				resetBuffersSet.updateBufferInfos(0, offsetInOutputModelData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output object data"<<std::endl;
				resetBuffersSet.updateBufferInfos(1, offsetInOutputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output material data"<<std::endl;
				resetBuffersSet.updateBufferInfos(2, offsetInOutputMaterialData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update offset in output indirect draw commands"<<std::endl;
				resetBuffersSet.updateBufferInfos(3, offsetInOutputElementsIndirectCommands, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update input material datax"<<std::endl;
				resetBuffersSet.updateBufferInfos(4, materialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update first index vertex"<<std::endl;
				resetBuffersSet.updateBufferInfos(5, drawCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				resetBuffersSet.updateBufferInfos(6, instanceBase, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);				
				//std::cout<<"update reset buffer set"<<std::endl;
				resetBuffersSet.updateDescriptorSet();
				DescriptorSet& cullingBatchingSet = GPUContext::instance().getDescriptorSets(cullingBatchingShader, 17, 1)[0];
				//std::cout<<"update object types"<<std::endl;
				cullingBatchingSet.updateBufferInfos(0, objectTypes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update objects"<<std::endl;
				cullingBatchingSet.updateBufferInfos(1, objects, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range objects : "<<objects[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(2, subMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range  submeshes : "<<subMeshes[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(3, modelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range model datas : "<<modelDatas[0].getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(4, materialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range material datas  : "<<materialDatas[0].getRange()<<std::endl;
				//std::cout<<"update output vertics"<<std::endl;
				cullingBatchingSet.updateBufferInfos(5, outputMaterialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output vertices : "<<outputVertexDatas[3].getVertexBuffer(0).getRange()<<std::endl;
				cullingBatchingSet.updateBufferInfos(6, outputModelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output indexes : "<<outputVertexDatas[3].getIndexBuffer(0).getRange()<<std::endl;
				//std::cout<<"model data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(7, outputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"range output model datas : "<<outputModelDatas[0].getRange()<<std::endl;
				//std::cout<<"object data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(8, offsetInOutputModelData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				/*std::cout<<"range output model datas : "<<outputModelDatas[0].getRange()<<std::endl;
				system("PAUSE");*/
				//std::cout<<"offset in output vertex"<<std::endl;
				cullingBatchingSet.updateBufferInfos(9, offsetInOutputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in output index"<<std::endl;
				cullingBatchingSet.updateBufferInfos(10, drawCount, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in model data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(11, outputElementsDrawIndirectCommand, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"offset in output object data"<<std::endl;
				cullingBatchingSet.updateBufferInfos(12, offsetInOutputElementsIndirectCommands, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"uniform buffer"<<std::endl;
				cullingBatchingSet.updateBufferInfos(13, offsetInOutputMaterialData, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				cullingBatchingSet.updateBufferInfos(14, instanceBase, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				cullingBatchingSet.updateBufferInfos(15, ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				cullingBatchingSet.updateBufferInfos(16, lodLevel, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				//std::cout<<"update culling ds"<<std::endl;
				cullingBatchingSet.updateDescriptorSet();
				bool hasDiffuseTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE).size() != 0;
				DescriptorSet& defaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, (hasDiffuseTextures) ? 4 : 3, 1)[0];
				defaultRenderingSet.updateBufferInfos(0, outputModelDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(1, outputMeshes, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				defaultRenderingSet.updateBufferInfos(2, outputMaterialDatas, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				if (hasDiffuseTextures ) {
					//std::cout<<"textures : "<<Texture::getAllTextures().size()<<std::endl;
					defaultRenderingSet.updateImageInfos(3, GPUContext::instance().getSharedTextures(entity::SubMesh::DIFFUSE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
				}
				defaultRenderingSet.updateDescriptorSet();
				bool hasSpecularTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR).size() != 0;
				if (hasSpecularTextures) {
					DescriptorSet& specularDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 1)[0];
					specularDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::SPECULAR), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					specularDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasNormalTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL).size() != 0;
				if (hasNormalTextures) {
					DescriptorSet& normalDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 2)[0];
					normalDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::NORMAL), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					normalDefaultRenderingSet.updateDescriptorSet();
				}

				bool hasMetalnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS).size() != 0;
				if (hasMetalnessTextures) {
					DescriptorSet& metalnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 3)[0];
					metalnessDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::METALNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					metalnessDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasRoughnessTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS).size() != 0;
				if (hasRoughnessTextures) {
					DescriptorSet& roughnessDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader,  1, 1, 4)[0];
					roughnessDefaultRenderingSet .updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::ROUGHNESS), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					roughnessDefaultRenderingSet .updateDescriptorSet();
				}
				bool hasAOTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::AO).size() != 0;
				if (hasAOTextures) {
					DescriptorSet& aoDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 5)[0];
					aoDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::AO), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					aoDefaultRenderingSet.updateDescriptorSet();
				}
				bool hasEmissiveTextures = GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE).size() != 0;
				if (hasEmissiveTextures) {
					DescriptorSet& emissiveDefaultRenderingSet = GPUContext::instance().getDescriptorSets(defaultRenderingShader, 1, 1, 6)[0];
					emissiveDefaultRenderingSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(entity::SubMesh::EMISSIVE), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					emissiveDefaultRenderingSet.updateDescriptorSet();
				}
				//std::cout<<"update descriptor sets : "<<vertexBufferDatas[0].getRange()<<","<<vertexBufferDatas[1].getRange()<<std::endl;
				if (hasDiffuseTextures) {
					DescriptorSet& vertexBufferSet = GPUContext::instance().getDescriptorSets(vertexBufferShader, 1, 1)[0];
					vertexBufferSet.updateImageInfos(0, GPUContext::instance().getSharedTextures(0), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
					vertexBufferSet.updateDescriptorSet();
				}	
			}
			//std::cout<<"set updated"<<std::endl;
		}
		bool RenderTarget::sameDepthStencil(const VkPipelineDepthStencilStateCreateInfo& a,
			const VkPipelineDepthStencilStateCreateInfo& b)
		{
			return a.depthTestEnable == b.depthTestEnable &&
				a.depthWriteEnable == b.depthWriteEnable &&
				a.depthCompareOp == b.depthCompareOp &&
				a.depthBoundsTestEnable == b.depthBoundsTestEnable &&
				a.stencilTestEnable == b.stencilTestEnable &&
				memcmp(&a.front, &b.front, sizeof(VkStencilOpState)) == 0 &&
				memcmp(&a.back, &b.back, sizeof(VkStencilOpState)) == 0 &&
				a.minDepthBounds == b.minDepthBounds &&
				a.maxDepthBounds == b.maxDepthBounds;
		}

		void RenderTarget::applyViewportAndScissor(VkCommandBuffer cmd) {
			for (unsigned int i = 0; i < viewports.size(); i++) {
				viewports[i].x = m_camera.getViewport().getPosition().x();
				viewports[i].y = m_camera.getViewport().getPosition().y();
				viewports[i].width = m_camera.getViewport().getSize().x();
				viewports[i].height = m_camera.getViewport().getSize().y();

				viewports[i].minDepth = 0.0f;
				viewports[i].maxDepth = 1.0f;
			}
			////////std::cout<<(m_view.getViewport().getSize().x == 800 && m_view.getViewport().getSize().y == 800)<<std::endl;
			vkCmdSetViewport(cmd, 0, viewports.size(), viewports.data());

			vkCmdSetScissor(cmd, 0, scissors.size(), scissors.data());
		}
		/*void RenderTarget::beginRenderPass(bool primary_subpass_content) {
			//std::cout<<"render pass depth ? "<<(depthTestEnabled)<<","<<(stencilTestEnabled)<<std::endl;
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			//std::cout<<"use depth test ?"<<(useDepthTest() || useStencilTest())<<std::endl;
			renderPassInfo.renderPass = (useDepthTest() || useStencilTest()) ? getRenderPass(1).getHandle() : getRenderPass(0).getHandle();
			renderPassInfo.framebuffer = (useDepthTest() || useStencilTest()) ? getFrameBuffers(1)[getImageIndex()].getHandle() : getFrameBuffers(0)[getImageIndex()].getHandle();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = getExtents();
			////////std::cout<<"render pass : "<<(m_view.getViewport().getSize().x == 800 && m_view.getViewport().getSize().y == 800)<<std::endl;

			//VkClearValue clrColor = {clearColor.r / 255.f,clearColor.g / 255.f, clearColor.b / 255.f, clearColor.a / 255.f};
			renderPassInfo.clearValueCount = 0;
			renderPassInfo.pClearValues = nullptr;


			vkCmdBeginRenderPass(commandPool.getHandle(getCurrentFrame()), &renderPassInfo, (primary_subpass_content) ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		}*/
		/*void RenderTarget::endRenderPass() {
			vkCmdEndRenderPass(commandPool.getHandle(getCurrentFrame()));
		}*/
		void RenderTarget::beginRecordCommandBuffer() {
			//std::cout<<"begin record command buffer : "<<getCurrentFrame()<<std::endl;
			needToUpdateCullBatchIndCmds = true;
			commandPool.beginRecordCommandBuffer(getCurrentFrame());
		}
		void RenderTarget::endRecordCommandBuffer() {
			//std::cout<<"end record command buffer : "<<getCurrentFrame()<<std::endl;
			commandPool.endRecordCommandBuffer(getCurrentFrame());
		}		
		void RenderTarget::applyCullingAndBatching() {
			//computeCommandPool.beginRecordCommandBuffer(getCurrentFrame());
			
			if (gameObjects.size() > 0) {

				updateBuffers();
				if (needToUpdateDescriptorSets) {
					updateDescriporSets();
					needToUpdateDescriptorSets = false;
				}

				
				if (ParticleSystemUpdater::instance(cv, mtx).isRunning()
					&& MorphAnimUpdater::instance(cv, mtx).isRunning()
					&& BoneAnimUpdater::instance(cv, mtx).isRunning()) {										
					
					std::unique_lock lock(mtx);	
					//std::cout<<"wait!"<<std::endl;				
					cv.wait(lock, [this]{return ParticleSystemUpdater::instance(cv, mtx).isSubmitReady() && MorphAnimUpdater::instance(cv, mtx).isSubmitReady() && BoneAnimUpdater::instance(cv, mtx).isSubmitReady();});
					/*std::unique_lock lock1(ParticleSystemUpdater::instance(cv, mtx).mtx);
					std::unique_lock lock2(MorphAnimUpdater::instance(cv, mtx).mtx);
					std::unique_lock lock3(BoneAnimUpdater::instance(cv, mtx).mtx);*/
					//std::cout<<"ready!"<<std::endl;
					ParticleSystemUpdater::instance(cv, mtx).setSubmitReady(false);
					BoneAnimUpdater::instance(cv, mtx).setSubmitReady(false);
					MorphAnimUpdater::instance(cv, mtx).setSubmitReady(false);
					//std::lock_guard<std::recursive_mutex> lock(getGlobalMutex());
					//std::cout<<"set sumbit ready!"<<std::endl;
					std::vector<VkFence> fences;
					for (unsigned int i = 0; i < 3; i++) {
						//std::cout<<"fence : "<<GPUContext::instance().getSharedFence(i)[0].getHandle()<<std::endl;
						fences.push_back(GPUContext::instance().getSharedFence(i)[0].getHandle());
					}
					//std::cout<<"wait for fences!"<<std::endl;
					vkWaitForFences(device.getDevice(), fences.size(),fences.data(), VK_TRUE, UINT64_MAX);
					vkResetFences(device.getDevice(), fences.size(), fences.data());
					//std::cout<<"fence unlocked!"<<std::endl;
					ParticleSystemUpdater::instance(cv, mtx).setReady(true);
					ParticleSystemUpdater::instance(cv, mtx).cv.notify_all();
					MorphAnimUpdater::instance(cv, mtx).setReady(true);
					MorphAnimUpdater::instance(cv, mtx).cv.notify_all();
					BoneAnimUpdater::instance(cv, mtx).setReady(true);
					BoneAnimUpdater::instance(cv, mtx).cv.notify_all();
					
					
					
				}
				
				
				//std::cout<<"bind pipeline : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
				vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(resetBuffersShader).getHandle());

				std::vector<VkDescriptorSet> sets;
				for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(resetBuffersShader).size(); i++) {
					sets.push_back(GPUContext::instance().getDescriptorSets(resetBuffersShader)[i][0].getHandle());
				}
				//std::cout<<"nb sets : "<<sets.size()<<std::endl;
				vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(resetBuffersShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
				cullingBatchingPc.currentFrame = getCurrentFrame();
				vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getComputePipeline(resetBuffersShader).getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingBatchingPC), &cullingBatchingPc);
				vkCmdDispatch(commandPool.getHandle(getCurrentFrame()), Material::getNbMaterials(), NB_PRIMITIVE_TYPES, 1);
				vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(cullingBatchingShader).getHandle());
				sets.clear();
				for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(cullingBatchingShader).size(); i++) {
					sets.push_back(GPUContext::instance().getDescriptorSets(cullingBatchingShader)[i][0].getHandle());
				}
				vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_COMPUTE, GPUContext::instance().getComputePipeline(cullingBatchingShader).getLayout(), 0, sets.size(), sets.data(), 0, 0);
				vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getComputePipeline(cullingBatchingShader).getLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingBatchingPC), &cullingBatchingPc);
				VkMemoryBarrier mem{};
				mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				mem.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0,
					1, &mem,
					0, nullptr,
					0, nullptr
				);
				//std::cout<<"dispatch : nbObjects  : "<<gameObjects.size()<<"nb material : "<<Material::getNbMaterials()<<std::endl;
				//std::cout<<"nb submeshes : "<<currentSubmeshesOffset<<std::endl;
				vkCmdDispatch(commandPool.getHandle(getCurrentFrame()), gameObjects.size(), Material::getNbMaterials(), NB_PRIMITIVE_TYPES);


				/*mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				mem.dstAccessMask =
					VK_ACCESS_SHADER_READ_BIT |            // SSBO dans VS/FS
					VK_ACCESS_INDIRECT_COMMAND_READ_BIT;   // draw indirect

				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
					VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					1, &mem,
					0, nullptr,
					0, nullptr
				);

				//computeCommandPool.endRecordCommandBuffer(getCurrentFrame());
				VkSubmitInfo submitInfo{};
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.signalSemaphoreCount = 1;
				submitInfo.pSignalSemaphores = &computeFinishedSemaphores[getCurrentFrame()].getHandle();
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &computeCommandPool.getHandle(getCurrentFrame());
				Device::QueueFamilyIndices indices = device.findQueueFamilies(device.getPhysicalDevice(), nullptr);
				//std::cout<<"submit!"<<std::endl;
				if (vkQueueSubmit(device.getQueue(indices.computeFamily.value(), 0), 1, &submitInfo, computeFences[getCurrentFrame()].getHandle()) != VK_SUCCESS) {
					throw std::runtime_error("Echec de l'envoi d'un command buffer!");
				}
				vkWaitForFences(device.getDevice(), 1, &computeFences[getCurrentFrame()].getHandle(), VK_TRUE, UINT64_MAX);
				vkResetFences(device.getDevice(), 1, &computeFences[getCurrentFrame()].getHandle());*/
				//std::cout<<"compute submitted"<<std::endl;
			}
		}
		void RenderTarget::draw(VertexBuffer& vb, RenderStates states) {
			//std::cout<<"update vb"<<std::endl;
			//std::cout<<"updated vb"<<std::endl;
			//std::cout<<"render target : "<<this<<std::endl;
			//std::cout<<"comamnd buffer : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
			Shader* shader = (states.shader == nullptr) ? &vertexBufferShader : states.shader;
			states.blendMode.updateIds();
			/*for (unsigned int i = 0; i < vb.getVertexCount(); i++) {
				vb[i].drawableDataId = cpuVertexBufferDatas[getCurrentFrame()].size();
			}*/
			vb.update(commandPool.getHandle(getCurrentFrame()), getCurrentFrame());
			//std::cout<<"bind pipeline ids : "<<shader->getId() * NB_PRIMITIVE_TYPES + vb.getPrimitiveType()<<","<<(depthStencilInfos.size()-1) * states.blendMode.nbBlendModes + states.blendMode.id<<std::endl;
			//std::cout<<"bind pipeline : "<<graphicsPipeline[shader->getId() * NB_PRIMITIVE_TYPES + vb.getPrimitiveType()][0][(depthStencilInfos.size()-1) * states.blendMode.nbBlendModes + states.blendMode.id].getHandle()<<std::endl;
			DepthStencilType depthStencilInfoId;
			if (!useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DepthStencilType::NODEPTHNOSTENCIL;
			} else if (useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DepthStencilType::DEPTHNOSTENCIL;
			} else if (!useDepthTest() && useStencilTest()) {
				depthStencilInfoId = DepthStencilType::NODEPTHSTENCIL;
			} else {
				depthStencilInfoId = DepthStencilType::DEPTHSTENCIL;
			}
			//std::cout<<"pirmitive type : "<<vb.getPrimitiveType()<<std::endl;
			vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(vb.getPrimitiveType(), *shader, states.blendMode, depthStencilInfoId).getHandle());
			//std::cout<<"pipeline bound"<<std::endl;


			VertexBufferData vertexBufferData;
			//cpuVertexBufferDatas[getCurrentFrame()].push_back(vertexBufferData);
			/*vertexBufferData.projMatrix = m_camera.getProjMatrix().getMatrix();
			vertexBufferData.viewMatrix = m_camera.getViewMatrix().getMatrix();
			vertexBufferData.modelMatrix = states.transform.getMatrix();

			vertexBufferData.textureIndex = (states.texture == nullptr) ? 0 : states.texture->getId();
			vertexBufferData.uvScale = (states.texture != nullptr) ? math::Vec2f(1.f / states.texture->getSize().x(), 1.f / states.texture->getSize().y()) : math::Vec2f(0.f, 0.f);
			vertexBufferData.uvOffset = math::Vec2f(0.f, 0.f);
			vertexBufferData.nbBuffers = (states.texture != nullptr) ? states.texture->getNbBuffers() : 0;
			vertexBufferData.drawableId = cpuVertexBufferDatas.size();*/
			vertexBufferPc.projMatrix = m_camera.getProjMatrix().getMatrix().transpose();
			/*std::cout<<"proj mat : "<<vertexBufferPc.projMatrix<<std::endl;
            system("PAUSE");*/         
			vertexBufferPc.viewMatrix = m_camera.getViewMatrix().getMatrix().transpose();
			vertexBufferPc.modelMatrix = states.transform.getMatrix().transpose();
			
			vertexBufferPc.textureIndex = (states.texture == nullptr) ? 0 : states.texture->getId();
			vertexBufferPc.uvScale = (states.texture != nullptr) ? math::Vec2f(1.f / states.texture->getSize().x(), 1.f / states.texture->getSize().y()) : math::Vec2f(0.f, 0.f);
			vertexBufferPc.uvOffset = math::Vec2f(0.f, 0.f);
			vertexBufferPc.nbBuffers = (states.texture != nullptr) ? states.texture->getNbBuffers() : 0;
			vertexBufferPc.currentFrame = getCurrentFrame();
			vertexBufferPc.currentImageIndex = getImageIndex();
			/*
			//std::cout<<"create copy buffers size : "<<getCurrentFrame()<<"size : "<<cpuVertexBufferDatas[getCurrentFrame()].size()<<","<<maxVertexBufferDataSizes[getCurrentFrame()]<<std::endl;
			if(cpuVertexBufferDatas[getCurrentFrame()].size() > maxVertexBufferDataSizes[getCurrentFrame()]) {
				//std::cout<<"create copy buffers size : "<<getCurrentFrame()<<"size : "<<cpuVertexBufferDatas[getCurrentFrame()].size()<<","<<maxVertexBufferDataSizes[getCurrentFrame()]<<std::endl;
				staggingVertexBufferDatas[getCurrentFrame()].create(sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
				vertexBufferDatas[getCurrentFrame()].create(sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				maxVertexBufferDataSizes[getCurrentFrame()] = cpuVertexBufferDatas[getCurrentFrame()].size();
				needToUpdateDescriptorSets = true;
			}
			//std::cout<<"copy vb datas"<<std::endl;
			staggingVertexBufferDatas[getCurrentFrame()].update(cpuVertexBufferDatas[getCurrentFrame()].data(), sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size());
			Buffer::copyBuffer(staggingVertexBufferDatas[getCurrentFrame()], vertexBufferDatas[getCurrentFrame()], sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), commandPool.getHandle(getCurrentFrame()));*/
			if (needToUpdateDescriptorSets) {
				std::cout<<"update descriptor sets"<<std::endl;
				updateDescriporSets();
				needToUpdateDescriptorSets = false;
			}
			VkBufferMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = vb.getVertexBuffer(getCurrentFrame()).getHandle();
			barrier.offset = 0;
			barrier.size = VK_WHOLE_SIZE;
			vkCmdPipelineBarrier(
				commandPool.getHandle(getCurrentFrame()),
				VK_PIPELINE_STAGE_TRANSFER_BIT,            // la copie doit être finie
				VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,        // avant que le shader lise
				0,
				0, nullptr,
				1, &barrier,
				0, nullptr
			);
			if (vb.getIndexCount() > 0) {
				barrier.buffer = vb.getIndexBuffer(getCurrentFrame()).getHandle();
				barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_TRANSFER_BIT,            // la copie doit être finie
					VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,        // avant que le shader lise
					0,
					0, nullptr,
					1, &barrier,
					0, nullptr
				);
				barrier.buffer = vertexBufferDatas[getCurrentFrame()].getHandle();
				barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_TRANSFER_BIT,            // la copie doit être finie
					VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,        // avant que le shader lise
					0,
					0, nullptr,
					1, &barrier,
					0, nullptr
				);
			}
			barrier.buffer = vertexBufferDatas[getCurrentFrame()].getHandle();
			vkCmdPipelineBarrier(
				commandPool.getHandle(getCurrentFrame()),
				VK_PIPELINE_STAGE_TRANSFER_BIT,            // la copie doit être finie
				VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,        // avant que le shader lise
				0,
				0, nullptr,
				1, &barrier,
				0, nullptr
			);
			//std::cout<<"begin render pass"<<std::endl;*/
			beginRendering();
			VkDeviceSize offsets[] = { 0, 0 };
			VkBuffer vertexBuffers[] = { vb.getVertexBuffer(getCurrentFrame()).getHandle()};
			vkCmdBindVertexBuffers(commandPool.getHandle(getCurrentFrame()), 0, 1, vertexBuffers, offsets);

			vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(vb.getPrimitiveType(), *shader, states.blendMode, depthStencilInfoId).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(VertexBufferPC), &vertexBufferPc);
			std::vector<VkDescriptorSet> shaderDescriptorSets;
			for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(*shader).size(); i++) {
				shaderDescriptorSets.push_back(GPUContext::instance().getDescriptorSets(*shader)[i][0].getHandle());
			}
			vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(vb.getPrimitiveType(), *shader, states.blendMode, depthStencilInfoId).getLayout(), 0, shaderDescriptorSets.size(), shaderDescriptorSets.data(), 0, nullptr);
			applyViewportAndScissor(commandPool.getHandle(getCurrentFrame()));
			//std::cout<<"use : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
			if (vb.getIndexCount() > 0) {
				vkCmdBindIndexBuffer(commandPool.getHandle(getCurrentFrame()), vb.getIndexBuffer(getCurrentFrame()).getHandle(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandPool.getHandle(getCurrentFrame()), vb.getIndexCount(), 1, 0, 0, 0);
			} else {
				vkCmdDraw(commandPool.getHandle(getCurrentFrame()), vb.getVertexCount(), 1, 0, 0);
			}
			endRendering();
		}
		void RenderTarget::applyComputeGraphicsBarrier() {
			VkMemoryBarrier mem{};
			mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			mem.dstAccessMask =
				VK_ACCESS_INDIRECT_COMMAND_READ_BIT |   // draw indirect
				VK_ACCESS_INDEX_READ_BIT |              // index buffer
				VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			/*Device::QueueFamilyIndices indexes = device.findQueueFamilies(device.getPhysicalDevice());// vertex buffer
			mem.srcQueueFamilyIndex = indexes.computeFamily.value();
			mem.dstQueueFamilyIndex = indexes.graphicsFamily.value();*/
			vkCmdPipelineBarrier(
				commandPool.getHandle(getCurrentFrame()),
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
				VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
				0,
				1, &mem,
				0, nullptr,
				0, nullptr
			);
		}
		void RenderTarget::applyComputeGraphicsBarrier(VertexBuffer& vb) {
			if (device.areMeshShadersSupported()) {
				VkMemoryBarrier2 barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
				barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

				VkDependencyInfo depInfo{};
				depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				depInfo.memoryBarrierCount = 1;
				depInfo.pMemoryBarriers = &barrier;

				vkCmdPipelineBarrier2(commandPool.getHandle(getCurrentFrame()), &depInfo);
			} else {				
				VkMemoryBarrier mem{};
				mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				mem.dstAccessMask =
					VK_ACCESS_INDIRECT_COMMAND_READ_BIT |   // draw indirect
					VK_ACCESS_INDEX_READ_BIT |              // index buffer
					VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				/*Device::QueueFamilyIndices indexes = device.findQueueFamilies(device.getPhysicalDevice());// vertex buffer
				mem.srcQueueFamilyIndex = indexes.computeFamily.value();
				mem.dstQueueFamilyIndex = indexes.graphicsFamily.value();*/
				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
					VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
					0,
					1, &mem,
					0, nullptr,
					0, nullptr
				);
			}
		}
		void RenderTarget::draw(CommandPool& commandPool, VertexBuffer& vb, RenderStates states) {
			///std::cout<<"update vb"<<std::endl;
			//std::cout<<"updated vb"<<std::endl;
			//std::cout<<"render target : "<<this<<std::endl;
			//std::cout<<"comamnd buffer : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
			Shader* shader = (states.shader == nullptr) ? &vertexBufferShader : states.shader;
			states.blendMode.updateIds();
			/*for (unsigned int i = 0; i < vb.getVertexCount(); i++) {
				vb[i].drawableDataId = cpuVertexBufferDatas[getCurrentFrame()].size();
			}*/
			//std::cout<<"bind pipeline ids : "<<shader->getId() * NB_PRIMITIVE_TYPES + vb.getPrimitiveType()<<","<<(depthStencilInfos.size()-1) * states.blendMode.nbBlendModes + states.blendMode.id<<std::endl;
			//std::cout<<"bind pipeline : "<<graphicsPipeline[shader->getId() * NB_PRIMITIVE_TYPES + vb.getPrimitiveType()][0][(depthStencilInfos.size()-1) * states.blendMode.nbBlendModes + states.blendMode.id].getHandle()<<std::endl;
			DepthStencilType depthStencilInfoId;
			if (!useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DepthStencilType::NODEPTHNOSTENCIL;
			} else if (useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DepthStencilType::DEPTHNOSTENCIL;
			} else if (!useDepthTest() && useStencilTest()) {
				depthStencilInfoId = DepthStencilType::NODEPTHSTENCIL;
			} else {
				depthStencilInfoId = DepthStencilType::DEPTHSTENCIL;
			}
			//vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(vb.getPrimitiveType(), *shader, states.blendMode, depthStencilInfoId).getHandle());
			//std::cout<<"pipeline bound"<<std::endl;


			VertexBufferData vertexBufferData;
			//cpuVertexBufferDatas[getCurrentFrame()].push_back(vertexBufferData);
			/*vertexBufferData.projMatrix = m_camera.getProjMatrix().getMatrix();
			vertexBufferData.viewMatrix = m_camera.getViewMatrix().getMatrix();
			vertexBufferData.modelMatrix = states.transform.getMatrix();

			vertexBufferData.textureIndex = (states.texture == nullptr) ? 0 : states.texture->getId();
			vertexBufferData.uvScale = (states.texture != nullptr) ? math::Vec2f(1.f / states.texture->getSize().x(), 1.f / states.texture->getSize().y()) : math::Vec2f(0.f, 0.f);
			vertexBufferData.uvOffset = math::Vec2f(0.f, 0.f);
			vertexBufferData.nbBuffers = (states.texture != nullptr) ? states.texture->getNbBuffers() : 0;
			vertexBufferData.drawableId = cpuVertexBufferDatas.size();*/
			/*vertexBufferPc.projMatrix = m_camera.getProjMatrix().getMatrix();
			vertexBufferPc.viewMatrix = m_camera.getViewMatrix().getMatrix();
			vertexBufferPc.modelMatrix = states.transform.getMatrix();

			vertexBufferPc.textureIndex = (states.texture == nullptr) ? 0 : states.texture->getId();
			vertexBufferPc.uvScale = (states.texture != nullptr) ? math::Vec2f(1.f / states.texture->getSize().x(), 1.f / states.texture->getSize().y()) : math::Vec2f(0.f, 0.f);
			vertexBufferPc.uvOffset = math::Vec2f(0.f, 0.f);
			vertexBufferPc.nbBuffers = (states.texture != nullptr) ? states.texture->getNbBuffers() : 0;
			vertexBufferPc.currentFrame = getCurrentFrame();
			vertexBufferPc.currentImageIndex = getImageIndex();*/
			/*
			//std::cout<<"create copy buffers size : "<<getCurrentFrame()<<"size : "<<cpuVertexBufferDatas[getCurrentFrame()].size()<<","<<maxVertexBufferDataSizes[getCurrentFrame()]<<std::endl;
			if(cpuVertexBufferDatas[getCurrentFrame()].size() > maxVertexBufferDataSizes[getCurrentFrame()]) {
				//std::cout<<"create copy buffers size : "<<getCurrentFrame()<<"size : "<<cpuVertexBufferDatas[getCurrentFrame()].size()<<","<<maxVertexBufferDataSizes[getCurrentFrame()]<<std::endl;
				staggingVertexBufferDatas[getCurrentFrame()].create(sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
				vertexBufferDatas[getCurrentFrame()].create(sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
				maxVertexBufferDataSizes[getCurrentFrame()] = cpuVertexBufferDatas[getCurrentFrame()].size();
				needToUpdateDescriptorSets = true;
			}
			//std::cout<<"copy vb datas"<<std::endl;
			staggingVertexBufferDatas[getCurrentFrame()].update(cpuVertexBufferDatas[getCurrentFrame()].data(), sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size());
			Buffer::copyBuffer(staggingVertexBufferDatas[getCurrentFrame()], vertexBufferDatas[getCurrentFrame()], sizeof(VertexBufferData) * cpuVertexBufferDatas[getCurrentFrame()].size(), commandPool.getHandle(getCurrentFrame()));*/
			/*if (needToUpdateDescriptorSets) {
				updateDescriporSets();
				needToUpdateDescriptorSets = false;
			}*/
			VkDeviceSize offsets[] = { 0, 0 };
			VkBuffer vertexBuffers[] = { vb.getVertexBuffer(0).getHandle()};
			vkCmdBindVertexBuffers(commandPool.getHandle(getCurrentFrame()), 0, 1, vertexBuffers, offsets);
			/*std::vector<VkDescriptorSet> shaderDescriptorSets;
			for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(*shader).size(); i++) {
				shaderDescriptorSets.push_back(GPUContext::instance().getDescriptorSets(*shader)[i][0].getHandle());
			}
			vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(vb.getPrimitiveType(), *shader, states.blendMode, depthStencilInfoId).getLayout(), 0, shaderDescriptorSets.size(), shaderDescriptorSets.data(), 0, nullptr);*/
			applyViewportAndScissor(commandPool.getHandle(getCurrentFrame()));
			//std::cout<<"use : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
			if (vb.getIndexCount() > 0) {
				vkCmdBindIndexBuffer(commandPool.getHandle(getCurrentFrame()), vb.getIndexBuffer(0).getHandle(), 0, VK_INDEX_TYPE_UINT32);
				vkCmdDrawIndexed(commandPool.getHandle(getCurrentFrame()), vb.getIndexCount(), 1, 0, 0, 0);
			} else {
				vkCmdDraw(commandPool.getHandle(getCurrentFrame()), vb.getVertexCount(), 1, 0, 0);
			}
		}
		void RenderTarget::draw(entity::PrimitiveType primitiveType, RenderStates states) {
			if (needToUpdateCullBatchIndCmds) {
				applyCullingAndBatching();
				//std::cout<<"draw"<<std::endl;
				needToUpdateCullBatchIndCmds = false;
			}
			Shader* shader = (states.shader == nullptr) ? &defaultRenderingShader : states.shader;
			viewProjInfos.primitiveType = primitiveType;
			/*std::cout<<"primitive type : "<<viewProjInfos.primitiveType<<std::endl;
			system("PAUSE");*/
			viewProjInfos.currentFrame = getCurrentFrame();
			indexesPC.currentImageIndex = getImageIndex();
			states.blendMode.updateIds();
			if (device.areMeshShadersSupported()) {
				VkMemoryBarrier2 barrier{};
				barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
				barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
				barrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
				barrier.dstStageMask = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT | VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
				barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

				VkDependencyInfo depInfo{};
				depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
				depInfo.memoryBarrierCount = 1;
				depInfo.pMemoryBarriers = &barrier;

				vkCmdPipelineBarrier2(commandPool.getHandle(getCurrentFrame()), &depInfo);
			} else {				
				VkMemoryBarrier mem{};
				mem.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				mem.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				mem.dstAccessMask =
					VK_ACCESS_INDIRECT_COMMAND_READ_BIT |   // draw indirect
					VK_ACCESS_INDEX_READ_BIT |              // index buffer
					VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
				/*Device::QueueFamilyIndices indexes = device.findQueueFamilies(device.getPhysicalDevice());// vertex buffer
				mem.srcQueueFamilyIndex = indexes.computeFamily.value();
				mem.dstQueueFamilyIndex = indexes.graphicsFamily.value();*/
				vkCmdPipelineBarrier(
					commandPool.getHandle(getCurrentFrame()),
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT |
					VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
					0,
					1, &mem,
					0, nullptr,
					0, nullptr
				);
			}
			beginRendering();
			DepthStencilType depthStencilInfoId;
			if (!useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = NODEPTHNOSTENCIL;
			} else if (useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DEPTHNOSTENCIL;
			} else if (!useDepthTest() && useStencilTest()) {
				depthStencilInfoId = NODEPTHSTENCIL;
			} else {
				depthStencilInfoId = DEPTHSTENCIL;
			}
			/*states.blendMode = BlendAlpha;
			states.blendMode.updateIds();
			std::cout<<"pipeline  : "<<primitiveType<<","<<shader->getId()<<","<<states.blendMode.id<<","<<depthStencilInfoId<<std::endl;
			std::cout<<"pipeline handle : "<<GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getHandle()<<std::endl;*/
			vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getHandle());
			if (device.areMeshShadersSupported()) {
				vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(ViewProjMatPC), &viewProjInfos);
			} else {
				vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewProjMatPC), &viewProjInfos);
			}			
			vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ViewProjMatPC), sizeof(IndexesPC), &indexesPC);
			if (!device.areMeshShadersSupported()) {
				VkBuffer vertexBuffers[] = { vertices[primitiveType].getVertexBuffer(0).getHandle()};
				VkDeviceSize offsets[] = { 0, 0 };
				vkCmdBindVertexBuffers(commandPool.getHandle(getCurrentFrame()), 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandPool.getHandle(getCurrentFrame()), vertices[primitiveType].getIndexBuffer(0).getHandle(), 0, VK_INDEX_TYPE_UINT32);
			}
			std::vector<VkDescriptorSet> shaderDescriptorSets;
			for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(*shader).size(); i++) {
				shaderDescriptorSets.push_back(GPUContext::instance().getDescriptorSets(*shader)[i][0].getHandle());
			}
			vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), 0, shaderDescriptorSets.size(), shaderDescriptorSets.data(), 0, nullptr);

			applyViewportAndScissor(commandPool.getHandle(getCurrentFrame()));
			if (device.areMeshShadersSupported()) {
				//std::cout<<"command pool : "<<commandPool.getHandle(getCurrentFrame())<<std::endl;
				vkCmdDrawMeshTasksEXT(commandPool.getHandle(getCurrentFrame()), MAX_TASKS, 1, 1);

			} else {
				vkCmdDrawIndexedIndirectCount(commandPool.getHandle(getCurrentFrame()), outputElementsDrawIndirectCommand[primitiveType*MAX_FRAMES_IN_FLIGHT+getCurrentFrame()].getHandle(), 0, drawCount[primitiveType*MAX_FRAMES_IN_FLIGHT+getCurrentFrame()].getHandle(), 0, MAX_DRAW_INDIRECT_COMMANDS, sizeof(DrawElementsIndirectCommand));
			}
			endRendering();
		}
		Fence& RenderTarget::getComputeFences(int currentFrame) {
			return computeFences[currentFrame];
		}
		Semaphore& RenderTarget::getComputeFinishedSemaphore(int currentFrame) {
			return computeFinishedSemaphores[currentFrame];
		}
		/*void RenderTarget::draw(unsigned int pipelineId, unsigned int depthStencilId) {
			for (unsigned int i = 0; i < drawables.size(); i++) {
				draw(drawables[i].getVertexBuffer().getPrimitiveType(), drawables[i].getRenderStates());
			}
		}*/
		void RenderTarget::draw(CommandPool& commandPool, entity::PrimitiveType primitiveType, RenderStates states) {
			Shader* shader = (states.shader == nullptr) ? &defaultRenderingShader : states.shader;
			/*viewProjInfos.primitiveType = primitiveType;
			viewProjInfos.currentFrame = getCurrentFrame();*/
			states.blendMode.updateIds();
			/*DepthStencilType depthStencilInfoId;
			if (!useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = NODEPTHNOSTENCIL;
			} else if (useDepthTest() && !useStencilTest()) {
				depthStencilInfoId = DEPTHNOSTENCIL;
			} else if (!useDepthTest() && useStencilTest()) {
				depthStencilInfoId = NODEPTHSTENCIL;
			} else {
				depthStencilInfoId = DEPTHSTENCIL;
			}*/
			/*if (needToUpdateDescriptorSets) {
				updateDescriporSets();
				needToUpdateDescriptorSets = false;
			}*/
			/*std::cout<<"pipeline ids primitive type : "<<primitiveType<<", shader : "<<shader->getId()<<",blend mode : "<<states.blendMode.id<<",depth stencil id : "<<depthStencilInfoId<<std::endl;
			std::cout<<"pipeline : "<<GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getHandle()<<std::endl;*/
			//vkCmdPushConstants(commandPool.getHandle(getCurrentFrame()), GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ViewProjMatPC), &viewProjInfos);

			//vkCmdBindPipeline(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getHandle());
			VkBuffer vertexBuffers[] = { vertices[primitiveType].getVertexBuffer(0).getHandle()};
			VkDeviceSize offsets[] = { 0, 0 };
			//std::cout<<"vertex count : "<<vertices[primitiveType].getVertexCount()<<std::endl;
			vkCmdBindVertexBuffers(commandPool.getHandle(getCurrentFrame()), 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandPool.getHandle(getCurrentFrame()), vertices[primitiveType].getIndexBuffer(0).getHandle(), 0, VK_INDEX_TYPE_UINT32);
			/*std::vector<VkDescriptorSet> shaderDescriptorSets;
			for (unsigned int i = 0; i < GPUContext::instance().getDescriptorSets(*shader).size(); i++) {
				shaderDescriptorSets.push_back(GPUContext::instance().getDescriptorSets(*shader)[i][0].getHandle());
			}
			vkCmdBindDescriptorSets(commandPool.getHandle(getCurrentFrame()), VK_PIPELINE_BIND_POINT_GRAPHICS, GPUContext::instance().getGraphicsPipeline(primitiveType, *shader, states.blendMode, depthStencilInfoId).getLayout(), 0, shaderDescriptorSets.size(), shaderDescriptorSets.data(), 0, nullptr);*/
			//std::cout<<"draw!"<<std::endl;
			applyViewportAndScissor(commandPool.getHandle(getCurrentFrame()));
			vkCmdDrawIndexedIndirectCount(commandPool.getHandle(getCurrentFrame()), outputElementsDrawIndirectCommand[primitiveType*MAX_FRAMES_IN_FLIGHT+getCurrentFrame()].getHandle(), 0, drawCount[primitiveType*MAX_FRAMES_IN_FLIGHT+getCurrentFrame()].getHandle(), 0, MAX_DRAW_INDIRECT_COMMANDS, sizeof(DrawElementsIndirectCommand));
		}
		void RenderTarget::setCamera(Camera camera)
		{				
			cullingInfo.frustrum.center = camera.getViewVolume().getCenter();
			cullingInfo.frustrum.size = camera.getViewVolume().getSize();
			cullingInfo.nbEntitiesTypes = entity::Entity::getNbEntitiesTypes();
			for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				ubo[i].update(&cullingInfo, sizeof(UBO));
			}
			viewProjInfos.projMatrix = camera.getProjMatrix().getMatrix().transpose();
			viewProjInfos.viewMatrix = camera.getViewMatrix().getMatrix().transpose();
			/*cullingBatchingPc.projMatrix = viewProjInfos.projMatrix;
			cullingBatchingPc.viewMatrix = viewProjInfos.viewMatrix;*/
			//std::cout<<"view matrix"<<viewProjInfos.viewMatrix<<std::endl<<"projMatrix : "<<viewProjInfos.projMatrix<<std::endl;
			m_camera = camera;
			ParticleSystemUpdater::instance(cv, mtx).setCamera(camera);
			needToUpdateDescriptorSets = true;
		}
		Camera& RenderTarget::getCamera() {
			return m_camera;
		}
		Camera& RenderTarget::getDefaultCamera() {
			return m_defaultCamera;
		}
		math::Vec4f RenderTarget::mapPixelToCoords(math::Vec4f point)
		{
			return mapPixelToCoords(point, getCamera());
		}


		math::Vec4f RenderTarget::mapPixelToCoords(math::Vec4f point, Camera& camera)
		{
			point[3] = 1;
			ViewportMatrix vpm;
			vpm.setViewport(math::Vec3f(m_camera.getViewport().getPosition().x(), m_camera.getViewport().getPosition().y(), 0.f)
				, math::Vec3f(m_camera.getViewport().getWidth(), m_camera.getViewport().getHeight(), 1.f));
			math::Vec4f coords = vpm.toNormalizedCoordinates(point);
			coords = m_camera.getProjMatrix().unProject(coords);
			coords = coords.normalizeToVec3();
			coords = m_camera.getViewMatrix().inverseTransform(coords);
			return coords;
		}

		math::Vec4f RenderTarget::mapCoordsToPixel(math::Vec4f point)
		{
			return mapCoordsToPixel(point, getCamera());
		}		
		math::Vec4f RenderTarget::mapCoordsToPixel(math::Vec4f point, Camera& view) {
			point[3] = 1;
			ViewportMatrix vpm;
			vpm.setViewport(math::Vec3f(m_camera.getViewport().getPosition().x(), m_camera.getViewport().getPosition().y(), 0.f),
				math::Vec3f(m_camera.getViewport().getWidth(), m_camera.getViewport().getHeight(), 1.f));
			//////std::cout<<"viewport matrix : "<<vpm.getMatrix()<<std::endl;
			math::Vec4f coords = m_camera.getViewMatrix().transform(point);
			//////std::cout<<"view matrix : "<<view.getViewMatrix().getMatrix()<<std::endl<<coords<<std::endl;
			coords = m_camera.getProjMatrix().project(coords);
			//////std::cout<<"projection matrix : "<<view.getProjMatrix().getMatrix()<<std::endl<<coords<<std::endl;
			coords = coords.normalizeToVec3();
			//////std::cout<<"n coords : "<<coords<<std::endl;
			coords = vpm.toViewportCoordinates(coords);
			//////std::cout<<"vp coords : "<<coords<<std::endl;
			return coords;
		}
		CommandPool& RenderTarget::getCommandPool() {
			return commandPool;
		}
		void RenderTarget::resetVertexBufferDatas() {
			cpuVertexBufferDatas[getCurrentFrame()].clear();
		}
		Texture& RenderTarget::getDepthStencilTexture() {
			return depthStencilTexture;
		}
		void RenderTarget::setDepthStencil(bool depthEnable, bool stencilEnable) {
			enableDepthTest = depthEnable;
			enableStencilTest = stencilEnable;
		}
		bool RenderTarget::useDepthTest() {
			return enableDepthTest;
		}
		bool RenderTarget::useStencilTest() {
			return enableStencilTest;
		}
	}
}
