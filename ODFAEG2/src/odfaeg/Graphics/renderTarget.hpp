#ifndef ODFAEG_RENDERTARGET_HPP
#define ODFAEG_RENDERTARGET_HPP
#include <vulkan/vulkan.hpp>
#include <set>
#include <vector>
#include <array>
#include <deque>
#include <odfaeg/config.hpp>
#include <condition_variable>
#include <iostream>
#include "../Math/vec.hpp"
#include "../Math/matrix.hpp"
#include "../Entity/primitiveType.hpp"
#include "device.hpp"
#include "shader.hpp"
#include "renderStates.hpp"
#include "blendMode.hpp"
#include "../Entity/color.hpp"
#include "commandPool.hpp"
#include "buffer.hpp"
#include "vertexBuffer.hpp"
#include "descriptor.hpp"
#include "pipeline.hpp"
#include "mesh.hpp"
#include "fence.hpp"
#include "semaphore.hpp"
#include "camera.hpp"
#include "renderPass.hpp"
#include "frameBuffer.hpp"
#include "texture.hpp"
#include "particleSystemUpdater.hpp"
#include "morphAnimUpdater.hpp"
#include "boneAnimUpdater.hpp"
#include "viewportMatrix.hpp"
#include "../Entity/grid.hpp"
namespace odfaeg {
	namespace graphic {		
		class RenderTarget {
		
		public:
			static std::vector<Mesh*>& getGameObjects() {
				static std::vector<Mesh*> gameObjects;
				return gameObjects;
			}
			enum BufferID {
				OBJECT_BUFFER, OBJECT_STAGGING_BUFFER, SUBMESHES_BUFFER, SUBMESHES_STAGGING_BUFFER, LOD_BUFFER, LOD_STAGGING_BUFFER, MODEL_DATA_BUFFER,
				STAGGING_MODEL_DATA_BUFFER, MATERIAL_DATA_BUFFER, STAGGING_MATERIAL_DATA_BUFFER, VERTEX_BUFFER, OUTPUT_MESHES, OUTPUT_MODELS, OUTPUT_MATERIALS, LINKED_LISTS, G_BUFFER, NB_BUFFERS
			};
			
			enum DepthStencilType {
				NODEPTHNOSTENCIL, DEPTHNOSTENCIL, DEPTHSTENCIL, NODEPTHSTENCIL
			};
			RenderTarget(Device& device, bool useDepthTest, bool useStencilTest);
			struct Offset {
				unsigned int previousOffsetVertex;
				unsigned int offsetVertex;    
				unsigned int instanceOffset;
				unsigned int clusterOffset;
				unsigned int submeshOffset;
				unsigned int offsetTaskData;
			};
			struct alignas(16) AABB {
				alignas(16) math::Vec3f center; //float _pad0; // vec3 + padding
				alignas(16) math::Vec3f size;   //float _pad1; // vec3 + padding
			};
			struct Object {
				AABB globalBounds;
				int id;
				int type;
				int subMeshesOffset;
				int modelDataOffset;
				int nbSubMeshes;
				int renderingType;
				int paddings[2];
			};
			struct LODLevelData {
				int index_offset;
				int index_count;
				int meshletOffset;
				int meshletCount;
			};
			struct alignas(16) SubMeshData {
				AABB globalBounds;
				int vertexOffset;
				int indexOffset;
				int primitiveType;
				int materialId;
				int nbVertices;
				int nbIndexes;
				int lodOffset;
				int id;
				int lodLevel;
				int objectId;
				int clusterOffset;
				int clusterCount;								
			};
			struct alignas(16) Cluster {
				AABB volume;
				int meshletOffset;
				int meshletCount;
				int id;
				int lodLevel;
				int leaf;
				int submeshId;				
				//int children[8];
				int pad[2];
			};
			struct CellData {
				AABB volume;
				int clusterId;
				int clusterOffset;
				int clusterCount;
				int pad;
			};
			struct Meshlet {
				unsigned int id;
				unsigned int vertexOffset;
				unsigned int indexOffset;
				unsigned int nbVertices;
				unsigned int nbIndexes;
				unsigned int minVertex;
				unsigned int maxVertex;
				alignas(16) math::Vec3f mins;
				alignas(16) math::Vec3f maxs;
				unsigned int clusterId;					
				unsigned int lod;	
				int voxel;
				int rendered;
				int pad; 
				bool operator==(const Meshlet& other) const {
					return id == other.id 
					&& mins.x() == other.mins.x()
					&& mins.y() == other.mins.y()
					&& mins.z() == other.mins.z()
					&& maxs.x() == other.maxs.x()
					&& maxs.y() == other.maxs.y()
					&& maxs.z() == other.maxs.z();
				}						
			};			
			struct ModelData {
				math::Matrix4f modelMatrix;
				math::Matrix4f shadowProjMatrix;
				math::Matrix4f borderMatrices;
			};
			struct MaterialData {
				/*math::Vec2f uvScale;
				math::Vec2f uvOffset;*/
				unsigned int diffuseTextureIndex;
				unsigned int specularTextureIndex;
				unsigned int normalTextureIndex;
				unsigned int metalnessTextureIndex;
				unsigned int roughnessTextureIndex;
				unsigned int aoTextureIndex;
				unsigned int emissiveTextureIndex;
				unsigned int materialType;
				unsigned int materialSet;
				unsigned int nbVertices;
				unsigned int nbIndexes;
				int instanceGroupId;
				unsigned int vertsInstanceSet;
				unsigned int materialId;
				unsigned int nbBuffers;
				int reflectable;
				int refractable;
			};	
			struct DrawElementsIndirectCommand {
				unsigned int index_count;
				unsigned int instance_count;
				unsigned int first_index;       // cf parametre offset de glDrawElements()
				unsigned int vertex_base;
				unsigned int instance_base;
			};
			struct MeshDrawCommand {
				unsigned int groupXCount;
				unsigned int groupYCount;
				unsigned int groupZCount;
			};
			struct TaskData {
				MeshDrawCommand meshDrawCommand;
				unsigned int baseInstance;
			};
			struct alignas(16) UBO {
				AABB frustrum;
				unsigned int nbEntitiesTypes;
				alignas(16) unsigned int gridCellCount[5];   
				alignas(16) math::Vec4f gridCellSize[5];
				alignas(16) math::Vec4f nbCellsPerRow[5];    
				alignas(16) math::Vec4f gridSize[5];   
				alignas(16) math::Vec4f gridPos[5];
				unsigned int pads[3];
			};
			struct ViewProjMatPC {
				math::Matrix4f projMatrix;
				math::Matrix4f viewMatrix;
				int primitiveType;
				int currentFrame;
			};
			struct IndexesPC {
            	int currentImageIndex;
			};
			struct VertexBufferData {
				math::Matrix4f projMatrix;
				math::Matrix4f viewMatrix;
				math::Matrix4f modelMatrix;				
				int textureIndex;
				math::Vec2f uvScale;
				math::Vec2f uvOffset;
				int nbBuffers;
			};
			struct VertexBufferPC {
			    math::Matrix4f projMatrix;
                math::Matrix4f viewMatrix;
                math::Matrix4f modelMatrix;                
                math::Vec2f uvScale;
                math::Vec2f uvOffset;
				int textureIndex;
                int nbBuffers;
				int currentFrame;
				int currentImageIndex;
			};
			struct CullingBatchingPC {
			    unsigned int currentFrame;				
			};
			
			inline static std::mutex mtx = std::mutex();
			inline static std::condition_variable cv = std::condition_variable();	
			void addGameObject(Mesh* objet);
			void removeGameObject(Mesh* object);
			void resetVertexBufferDatas();
			void createDescriptorAndPipelines();
			void setTypesToRender(std::string expression, unsigned int currentFrame);
			void applyCullingAndBatching();
			template <typename D>
			void draw(D& drawable, RenderStates states=RenderStates::Default);
			void draw(VertexBuffer& vb, RenderStates states=RenderStates::Default);
			void draw(CommandPool& commandPool, VertexBuffer& vb, RenderStates states=RenderStates::Default);
			void draw(entity::PrimitiveType primitiveType, RenderStates states = RenderStates::Default);
			void draw(CommandPool& commandPool, entity::PrimitiveType primitiveType, RenderStates states);
			void setCamera(Camera camera);
			virtual Image& getRenderingImage() = 0;
			Camera& getCamera();
			Camera& getDefaultCamera();
			std::vector<VkPipelineDepthStencilStateCreateInfo> getDepthStencilInfos();
			math::Vec4f mapPixelToCoords(math::Vec4f point);


			math::Vec4f mapPixelToCoords(math::Vec4f point, Camera& camera);

			math::Vec4f mapCoordsToPixel(math::Vec4f point);
			Fence& getComputeFences(int currentFrame);
			Semaphore& getComputeFinishedSemaphore(int currentFrame);

			math::Vec4f mapCoordsToPixel(math::Vec4f point, Camera& view);
			virtual void beginRenderPass(bool secondaryCommandBuffers=false) = 0;
			virtual void endRenderPass() = 0;
			virtual void beginRendering(bool secondaryCommandBuffers=false, int viewIndex=-1) = 0;
			virtual void endRendering() = 0;
			virtual std::uint32_t getSwapchainImagesCount() = 0;
			void beginRecordCommandBuffer();
			void endRecordCommandBuffer();
			virtual uint32_t getCurrentFrame() = 0;
			virtual RenderPass& getRenderPass(unsigned int id=0) = 0;
			virtual uint32_t getRenderPassesCount() = 0;
			virtual bool isDynamicRendering() = 0;
			virtual std::vector<FrameBuffer>& getFrameBuffers(unsigned int id=0) = 0;
			virtual VkFormat& getImageFormat() = 0;
			virtual VkExtent2D getExtents() = 0;
			virtual void clear(const entity::Color& color) = 0;
			virtual uint32_t getImageIndex() = 0;
			virtual math::Vector2u getSize() const = 0;
			virtual VkSurfaceKHR getSurface() = 0;
			virtual void submit(bool lastSubmit = false, std::vector<VkSemaphore> signalSemaphores = std::vector<VkSemaphore>(),
                                                        std::vector<VkSemaphore> waitSemaphores = std::vector<VkSemaphore>(), std::vector<VkPipelineStageFlags> waitStages = std::vector<VkPipelineStageFlags>(),
                                                        std::vector<uint64_t> signalValues = std::vector<uint64_t>(),
                                                        std::vector<uint64_t> waitValues = std::vector<uint64_t>(), std::vector<VkFence>fences = std::vector<VkFence>(), unsigned int queueIndex = 0, bool resetFence = true, bool resetFences = true, VkFence fenceToSubmit = nullptr) = 0;
			bool useDepthTest();
			bool useStencilTest();
			virtual bool isDepthOnly() = 0;
			virtual std::uint32_t& getViewMask() = 0;
			void setDepthStencil(bool enableDepth, bool enableStencil);
			CommandPool& getCommandPool();
			void updateBuffers();
			~RenderTarget();
			bool sameDepthStencil(const VkPipelineDepthStencilStateCreateInfo& a,
				const VkPipelineDepthStencilStateCreateInfo& b);
			void enableDepthStencil(bool enableDepth, bool enableStencil);
			Texture& getDepthStencilTexture();
			void applyComputeGraphicsBarrier();
			void applyComputeGraphicsBarrier(VertexBuffer& vertexBuffer);
			int getId();
			static unsigned int getNbRenderTarget();			
			protected:
				void initialize();
			private :				
			void createSynchPrimitives();
			void applyViewportAndScissor(VkCommandBuffer cmd);
			ViewProjMatPC viewProjInfos;
			VertexBufferPC vertexBufferInfos;
			UBO cullingInfo;

			void updateDescriporSets();			
			std::deque<Buffer>& objects;
			std::deque<Buffer>& staggingObjects;
			std::deque<Buffer>& subMeshes;
			std::deque<Buffer>& staggingSubMeshes;
			std::deque<Buffer> staggingMeshlets;
			std::deque<Buffer> staggingClusters;
			std::deque<Buffer> staggingGridCells;
			std::deque<Buffer>& staggingLODLevel;
			std::deque<Buffer>& lodLevel;
			std::deque<VertexBuffer>& vertices;
			std::deque<VertexBuffer> outputVertices;
			std::deque<Buffer>& modelDatas;
			std::deque<Buffer>& staggingModelDatas;
			std::deque<Buffer>& materialDatas;
			std::deque<Buffer>& staggingMaterialDatas;
			std::deque<Buffer> vertexBufferDatas;
			std::deque<Buffer> staggingVertexBufferDatas;
			std::deque<Buffer> objectTypes;
			std::deque<Buffer> staggingObjectTypes;
			std::deque<Buffer> outputObjectDatas;
			std::deque<Buffer>& outputModelDatas;
			std::deque<Buffer>& outputMaterialDatas;
			std::deque<Buffer> outputElementsDrawIndirectCommand;
			std::deque<Buffer> outputTaskDatas;
			std::deque<Buffer> inputMeshlets;
			std::deque<Buffer> inputCellDatas;
			std::deque<Buffer> inputClusters;
			std::deque<Buffer> outputClusters;
			std::deque<Buffer> offsetInOutputModelData;
			std::deque<Buffer> offsetInOutputObjectData;
			std::deque<Buffer> offsetInOutputMaterialData;
			std::deque<Buffer> offsetInOutputElementsIndirectCommands;
			std::deque<Buffer> offsetInOutputTaskDatas;
			std::deque<Buffer> offsetInOutputMeshes;
			std::deque<Buffer> offsetBuffer;
			std::deque<Buffer> outputVertexBuffer;
			std::deque<Buffer> gridCellBuffer;			
			std::deque<Buffer> instanceBase;
			std::deque<Buffer> ubo;
			std::deque<Buffer> drawCount;
			std::deque<Buffer> taskCount;
			std::deque<Buffer>& outputMeshes;			
			std::array<std::vector<VertexBufferData>, MAX_FRAMES_IN_FLIGHT> cpuVertexBufferDatas;
			VertexBufferPC vertexBufferPc;			
			inline static const unsigned int MAX_VERTS = 255u;
			inline static const unsigned int MAX_PRIMS = 85u;
			unsigned int totalMeshlets = 0;
			unsigned int totalClusters = 0;
			unsigned int totalSubMeshes = 0;
			inline static bool needToUpdateBuffers = false;
			Camera m_defaultCamera, m_camera;
			std::vector<Mesh*>& gameObjects;
			inline static unsigned int currentSubmeshesOffset = 0;
			inline static unsigned int currentModelDataOffset = 0;
			inline static unsigned int currentMeshletsOffset = 0;
			inline static unsigned int currentClustersOffset = 0;
			inline static std::array<unsigned int, NB_PRIMITIVE_TYPES> currentVertexOffset = {};
			inline static std::array<unsigned int, NB_PRIMITIVE_TYPES> currentIndexOffset = {};
			inline static const unsigned int MAX_DRAW_INDIRECT_COMMANDS = 10000, MAX_TASKS = 10000;
			std::array<unsigned int, MAX_FRAMES_IN_FLIGHT> maxVertexBufferDataSizes = {};
			CommandPool commandPool, computeCommandPool;
			Shader defaultRenderingShader, cullingBatchingShader, resetBuffersShader, vertexBufferShader;
			bool depthTestEnabled, stencilTestEnabled;
			Device& device;
			std::vector<VkPipelineDepthStencilStateCreateInfo> depthStencilInfos;
			inline static std::vector<RenderTarget*> registeredRenderTargets = std::vector<RenderTarget*>();
			bool needToUpdateDescriptorSets;
			std::array<VkRect2D, MAX_SCISSORS_AND_VIEWPORTS> scissors;
			std::array<VkViewport, MAX_SCISSORS_AND_VIEWPORTS> viewports;
			std::vector<Fence> computeFences;
			std::vector<Semaphore> computeFinishedSemaphores;
			CullingBatchingPC cullingBatchingPc;
			Texture depthStencilTexture;
			bool enableDepthTest, enableStencilTest;
			unsigned int id;
			bool needToUpdateCullBatchIndCmds;
			IndexesPC indexesPC;
			PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
			entity::GridMap<Meshlet> meshletsGrid;					
		};
		class Drawable {
            public :
            virtual void draw(RenderTarget& renderTarget, RenderStates states) = 0;
        };
        template <typename D>
        void RenderTarget::draw(D& drawable, RenderStates states) {
            drawable.draw(*this, states);
        }
	}
}
#endif