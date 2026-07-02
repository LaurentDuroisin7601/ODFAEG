set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Graphics)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Graphics)
#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")

add_library(odfaeg-graphics-mod)
target_compile_definitions(odfaeg-graphics-mod PUBLIC
    ODFAEG_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}"
)
target_sources(odfaeg-graphics-mod
		PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES
		BASE_DIRS ${SRCROOT}
		FILES
		${SRCROOT}/primitiveType.ixx
		${SRCROOT}/color.ixx
		${SRCROOT}/debug.ixx
		${SRCROOT}/instance.ixx
		${SRCROOT}/device.ixx
		${SRCROOT}/buffer.ixx
		${SRCROOT}/image.ixx
		${SRCROOT}/commandPool.ixx
		${SRCROOT}/blendMode.ixx
		${SRCROOT}/rect.ixx
		${SRCROOT}/imageLoader.ixx
		${SRCROOT}/texture.ixx
		${SRCROOT}/shader.ixx
		${SRCROOT}/renderStates.ixx
		${SRCROOT}/transformable.ixx
		${SRCROOT}/vertex.ixx
		${SRCROOT}/material.ixx
		${SRCROOT}/vertexBuffer.ixx
		${SRCROOT}/descriptor.ixx
		${SRCROOT}/pipeline.ixx
		${SRCROOT}/renderPass.ixx
		${SRCROOT}/frameBuffer.ixx
		${SRCROOT}/fence.ixx
		${SRCROOT}/semaphore.ixx
		${SRCROOT}/gameObject.ixx
		${SRCROOT}/projMatrix.ixx
		${SRCROOT}/viewMatrix.ixx
		${SRCROOT}/viewportMatrix.ixx
		${SRCROOT}/camera.ixx
		${SRCROOT}/renderTarget.ixx
		${SRCROOT}/renderTexture.ixx
		${SRCROOT}/swapchain.ixx
		${SRCROOT}/renderWindow.ixx
		${SRCROOT}/shape.ixx
		${SRCROOT}/rectangleShape.ixx
		${SRCROOT}/tile.ixx
		${SRCROOT}/model.ixx
		${SRCROOT}/modelLoader.ixx
		${SRCROOT}/assimpHelper.ixx
		${SRCROOT}/gpuContext.ixx
		${SRCROOT}/particle.ixx
		${SRCROOT}/emittors.ixx
		${SRCROOT}/particleSystem.ixx
		${SRCROOT}/particleSystemUpdater.ixx
		${SRCROOT}/morphAnim.ixx
		${SRCROOT}/morphAnimUpdater.ixx
		${SRCROOT}/bone.ixx
		${SRCROOT}/animation.ixx
		${SRCROOT}/animator.ixx
		${SRCROOT}/boneAnimUpdater.ixx
		${SRCROOT}/linkedListRenderer.ixx
		${SRCROOT}/shadowRenderer.ixx
		${SRCROOT}/cube.ixx
		${SRCROOT}/plane.ixx
)
target_sources(odfaeg-graphics-mod PRIVATE
	${SRCROOT}/model.cpp
	${SRCROOT}/tile.cpp
    ${SRCROOT}/color.cpp
    ${SRCROOT}/debug.cpp
    ${SRCROOT}/instance.cpp
    ${SRCROOT}/device.cpp
    ${SRCROOT}/buffer.cpp
    ${SRCROOT}/image.cpp
    ${SRCROOT}/commandPool.cpp
    ${SRCROOT}/blendMode.cpp
    ${SRCROOT}/imageLoader.cpp
    ${SRCROOT}/texture.cpp
    ${SRCROOT}/shader.cpp
    ${SRCROOT}/renderStates.cpp
    ${SRCROOT}/transformable.cpp
    ${SRCROOT}/vertex.cpp
    ${SRCROOT}/material.cpp
    ${SRCROOT}/vertexBuffer.cpp
    ${SRCROOT}/descriptor.cpp
    ${SRCROOT}/pipeline.cpp
    ${SRCROOT}/renderPass.cpp
    ${SRCROOT}/frameBuffer.cpp
    ${SRCROOT}/fence.cpp
    ${SRCROOT}/semaphore.cpp
    ${SRCROOT}/gameObject.cpp
    ${SRCROOT}/projMatrix.cpp
    ${SRCROOT}/camera.cpp
    ${SRCROOT}/viewportMatrix.cpp
    ${SRCROOT}/viewMatrix.cpp
    ${SRCROOT}/renderTarget.cpp
    ${SRCROOT}/renderTexture.cpp
    ${SRCROOT}/swapchain.cpp
    ${SRCROOT}/renderWindow.cpp
    ${SRCROOT}/shape.cpp
    ${SRCROOT}/rectangleShape.cpp
	${SRCROOT}/model.cpp
	${SRCROOT}/modelLoader.cpp	
	${SRCROOT}/gpuContext.cpp
	${SRCROOT}/particleSystem.cpp
	${SRCROOT}/particleSystemUpdater.cpp
	${SRCROOT}/morphAnim.cpp
	${SRCROOT}/morphAnimUpdater.cpp
	${SRCROOT}/bone.cpp
	${SRCROOT}/animation.cpp
	${SRCROOT}/animator.cpp
	${SRCROOT}/boneAnimUpdater.cpp
	${SRCROOT}/linkedListRenderer.cpp
	${SRCROOT}/shadowRenderer.cpp
	${SRCROOT}/cube.cpp
	${SRCROOT}/plane.cpp
)
target_compile_features(odfaeg-graphics-mod PUBLIC cxx_std_20)
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/meshoptimizer/src)
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/vma)
file(GLOB_RECURSE MESH_OPTIMIZER_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/external/meshoptimizer/src/*.cpp")
add_library(meshoptimizer STATIC
    ${MESH_OPTIMIZER_SRC}
)
target_link_libraries(odfaeg-graphics-mod PRIVATE meshoptimizer)
add_library(vma ${PROJECT_SOURCE_DIR}/external/vma/vk_mem_alloc.cpp)
target_link_libraries(odfaeg-graphics-mod PRIVATE vma)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
find_package(PkgConfig REQUIRED MODULE)
pkg_check_modules(VULKAN REQUIRED vulkan)

include_directories(${VULKAN_INCLUDE_DIRS})
link_directories(${VULKAN_LIBRARY_DIRS})
target_link_libraries(odfaeg-graphics-mod PUBLIC odfaeg-window-mod odfaeg-physics-mod odfaeg-entity-mod odfaeg-entity-mod odfaeg-math-mod odfaeg-core-mod PRIVATE ${VULKAN_LIBRAIRIES})

find_package(assimp CONFIG REQUIRED)
#find_package(freetype CONFIG REQUIRED)
find_package(shaderc CONFIG REQUIRED)
target_link_libraries(odfaeg-graphics-mod PRIVATE assimp::assimp
shaderc::shaderc)
target_include_directories(odfaeg-graphics-mod PRIVATE ${stb_INCLUDE_DIRS})
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external)
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/entt)
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/gli)
add_library(odfaeg-graphics STATIC)
target_include_directories(odfaeg-graphics-mod
		PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)
target_sources(odfaeg-graphics
		PUBLIC
		FILE_SET HEADERS TYPE HEADERS
		BASE_DIRS ${INCROOT}
		FILES ${HEADERS}
)

target_sources(odfaeg-graphics PRIVATE ${SRC})
target_link_libraries(odfaeg-graphics PRIVATE odfaeg-graphics-mod)
install(
		TARGETS odfaeg-graphics
		EXPORT odfaeg-targets
		FILE_SET HEADERS DESTINATION include
)
install(
		TARGETS odfaeg-graphics-mod	
		vma
		meshoptimizer
		EXPORT odfaeg-targets
		FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Graphics
)
install(
		DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-graphics-mod.dir/
		DESTINATION share/odfaeg/pcm/Graphics
		FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)


