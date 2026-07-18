set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Graphics)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Graphics)
#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
add_library(odfaeg-graphics-mod)
target_compile_definitions(odfaeg-graphics-mod PUBLIC
    ODFAEG_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}"
)
file(GLOB_RECURSE SHADERC_SRC CONFIGURE_DEPENDS 
    "${CMAKE_SOURCE_DIR}/Shader/*.vert"
    "${CMAKE_SOURCE_DIR}/Shader/*.frag"
    "${CMAKE_SOURCE_DIR}/Shader/*.comp"
    "${CMAKE_SOURCE_DIR}/Shader/*.mesh"
    "${CMAKE_SOURCE_DIR}/Shader/*.task")
file(GLOB_RECURSE SHADERC_SPV CONFIGURE_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/Shader/*.spv")
set(SPV_OUTPUTS "")
foreach(SHADER_SRC ${SHADERC_SRC})
    # Nom du fichier SPIR-V généré
    get_filename_component(SHADER_NAME ${SHADER_SRC} NAME)
    set(SHADER_SPV "${CMAKE_BINARY_DIR}/Shaders/${SHADER_NAME}.spv")

    add_custom_command(
        OUTPUT ${SHADER_SPV}
        COMMAND ${GLSLANG_VALIDATOR} --target-env spirv1.6 -V ${SHADER_SRC} -o ${SHADER_SPV}
        DEPENDS ${SHADER_SRC}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )

    list(APPEND SPV_OUTPUTS ${SHADER_SPV})
endforeach()

# Cible pour compiler tous les shaders
add_custom_target(compile_shaders ALL DEPENDS ${SPV_OUTPUTS})
target_sources(odfaeg-graphics-mod PRIVATE ${SPV_OUTPUTS})
target_sources(odfaeg-graphics-mod
		PUBLIC
		FILE_SET cxx_modules TYPE CXX_MODULES
		BASE_DIRS ${SRCROOT}
		FILES		
		${SRCROOT}/debug.ixx
		${SRCROOT}/instance.ixx
		${SRCROOT}/device.ixx
		${SRCROOT}/commandPool.ixx
		${SRCROOT}/buffer.ixx
		${SRCROOT}/image.ixx		
		${SRCROOT}/blendMode.ixx		
		${SRCROOT}/imageLoader.ixx
		${SRCROOT}/texture.ixx
		${SRCROOT}/shader.ixx
		${SRCROOT}/renderStates.ixx                
		${SRCROOT}/material.ixx
		${SRCROOT}/vertexBuffer.ixx
		${SRCROOT}/descriptor.ixx
		${SRCROOT}/pipeline.ixx
		${SRCROOT}/renderPass.ixx
		${SRCROOT}/frameBuffer.ixx
		${SRCROOT}/fence.ixx
		${SRCROOT}/semaphore.ixx		
		${SRCROOT}/projMatrix.ixx
		${SRCROOT}/viewMatrix.ixx
		${SRCROOT}/viewportMatrix.ixx
		${SRCROOT}/camera.ixx
		${SRCROOT}/gpuContext.ixx
		${SRCROOT}/renderTarget.ixx
		${SRCROOT}/renderTexture.ixx
		${SRCROOT}/swapchain.ixx		
		${SRCROOT}/shape.ixx
		${SRCROOT}/rectangleShape.ixx
		${SRCROOT}/modelLoader.ixx		
		${SRCROOT}/particleSystemUpdater.ixx
		${SRCROOT}/morphAnimUpdater.ixx		
		${SRCROOT}/boneAnimUpdater.ixx
		${SRCROOT}/linkedListRenderer.ixx
		${SRCROOT}/envMapRenderer.ixx
		${SRCROOT}/shadowRenderer.ixx
		${SRCROOT}/renderWindow.ixx
		${SRCROOT}/mesh.ixx
)
target_sources(odfaeg-graphics-mod PRIVATE
    ${SRCROOT}/debug.cpp
    ${SRCROOT}/instance.cpp
    ${SRCROOT}/commandPool.cpp
    ${SRCROOT}/device.cpp
    ${SRCROOT}/buffer.cpp
    ${SRCROOT}/image.cpp    
    ${SRCROOT}/blendMode.cpp
    ${SRCROOT}/imageLoader.cpp
    ${SRCROOT}/texture.cpp
    ${SRCROOT}/shader.cpp
    ${SRCROOT}/renderStates.cpp    
    ${SRCROOT}/material.cpp
    ${SRCROOT}/vertexBuffer.cpp
    ${SRCROOT}/descriptor.cpp
    ${SRCROOT}/pipeline.cpp
    ${SRCROOT}/renderPass.cpp
    ${SRCROOT}/frameBuffer.cpp
    ${SRCROOT}/fence.cpp
    ${SRCROOT}/semaphore.cpp
    ${SRCROOT}/projMatrix.cpp
    ${SRCROOT}/camera.cpp
    ${SRCROOT}/viewportMatrix.cpp
    ${SRCROOT}/viewMatrix.cpp
    ${SRCROOT}/gpuContext.cpp 
    ${SRCROOT}/renderTarget.cpp
    ${SRCROOT}/renderTexture.cpp
    ${SRCROOT}/swapchain.cpp    
    ${SRCROOT}/shape.cpp
    ${SRCROOT}/rectangleShape.cpp
    ${SRCROOT}/modelLoader.cpp    
    ${SRCROOT}/particleSystemUpdater.cpp
    ${SRCROOT}/morphAnimUpdater.cpp
    ${SRCROOT}/boneAnimUpdater.cpp
    ${SRCROOT}/linkedListRenderer.cpp
    ${SRCROOT}/shadowRenderer.cpp
    ${SRCROOT}/envMapRenderer.cpp
    ${SRCROOT}/renderWindow.cpp
    ${SRCROOT}/mesh.cpp
)
target_compile_features(odfaeg-graphics-mod PUBLIC cxx_std_20)

target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/vma)
#target_include_directories(odfaeg-graphics-mod PRIVATE ${VCPKG_INSTALLED_DIR}/x64-windows/include)
#target_include_directories(odfaeg-graphics-mod PRIVATE ${Vulkan_INCLUDE_DIRS})
add_library(vma ${PROJECT_SOURCE_DIR}/external/vma/vk_mem_alloc.cpp)
target_link_libraries(odfaeg-graphics-mod PRIVATE vma)
#set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
pkg_check_modules(FREETYPE REQUIRED freetype2)
pkg_check_modules(SHADERC REQUIRED shaderc)
pkg_check_modules(ASSIMP REQUIRED assimp)
#find_package(Vulkan REQUIRED)
target_include_directories(odfaeg-window-mod PRIVATE ${VULKAN_INCLUDE_DIRS})
target_link_libraries(odfaeg-graphics-mod PUBLIC odfaeg-window-mod odfaeg-physics-mod odfaeg-entity-mod odfaeg-entity-mod odfaeg-math-mod odfaeg-core-mod PUBLIC ${VULKAN_LIBRARIES})

#find_package(assimp CONFIG REQUIRED)
#find_package(freetype CONFIG REQUIRED)
#find_package(shaderc CONFIG REQUIRED)
target_include_directories(odfaeg-graphics-mod PRIVATE ${ASSIMP_INCLUDE_DIRS})
target_link_directories(odfaeg-graphics-mod PRIVATE ${ASSIMP_LIBRARY_DIRS})
target_link_libraries(odfaeg-graphics-mod PRIVATE ${ASSIMP_LIBRARIES}
${SHADERC_LIBRARIES})
target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/stb)
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


