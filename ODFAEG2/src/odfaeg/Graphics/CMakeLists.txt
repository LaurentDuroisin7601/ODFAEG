set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Graphics)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Graphics)
#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS_COMMON CONFIGURE_DEPENDS "${INCROOT}/*.hpp")
add_library(odfaeg-graphics STATIC)
file(GLOB_RECURSE SHADERC_SRC CONFIGURE_DEPENDS 
    "${CMAKE_SOURCE_DIR}/Shader/*.vert"
    "${CMAKE_SOURCE_DIR}/Shader/*.frag"
    "${CMAKE_SOURCE_DIR}/Shader/*.comp"
    "${CMAKE_SOURCE_DIR}/Shader/*.mesh"
    "${CMAKE_SOURCE_DIR}/Shader/*.task"
	"${CMAKE_SOURCE_DIR}/Shader/*.rgen"
	"${CMAKE_SOURCE_DIR}/Shader/*.rmiss"
	"${CMAKE_SOURCE_DIR}/Shader/*.rchit")
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
set(ODFAEG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
target_compile_definitions(odfaeg-graphics PUBLIC
    ODFAEG_INSTALL_DIR="${ODFAEG_INSTALL_DIR}"
)
# Cible pour compiler tous les shaders
add_custom_target(compile_shaders ALL DEPENDS ${SPV_OUTPUTS})
target_sources(odfaeg-graphics 
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS_COMMON}
)
if(ODFAEG_USE_MODULES)   
	add_library(odfaeg-graphics-mod STATIC)
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
			${SRCROOT}/iComponent.ixx
			${SRCROOT}/widget.ixx
			${SRCROOT}/iRenderer.ixx
			${SRCROOT}/renderGraph.ixx
			${SRCROOT}/componentManager.ixx
	)
	target_sources(odfaeg-graphics-mod PRIVATE
		${SRCROOT}/debug.inl
		${SRCROOT}/instance.inl
		${SRCROOT}/commandPool.inl
		${SRCROOT}/device.inl
		${SRCROOT}/buffer.inl
		${SRCROOT}/image.inl    
		${SRCROOT}/blendMode.inl
		${SRCROOT}/imageLoader.inl
		${SRCROOT}/texture.inl
		${SRCROOT}/shader.inl
		${SRCROOT}/renderStates.inl
		${SRCROOT}/material.inl
		${SRCROOT}/vertexBuffer.inl
		${SRCROOT}/descriptor.inl
		${SRCROOT}/pipeline.inl
		${SRCROOT}/renderPass.inl
		${SRCROOT}/frameBuffer.inl
		${SRCROOT}/fence.inl
		${SRCROOT}/semaphore.inl
		${SRCROOT}/projMatrix.inl
		${SRCROOT}/camera.inl
		${SRCROOT}/viewportMatrix.inl
		${SRCROOT}/viewMatrix.inl
		${SRCROOT}/gpuContext.inl
		${SRCROOT}/renderTarget.inl
		${SRCROOT}/renderTexture.inl
		${SRCROOT}/swapchain.inl 
		${SRCROOT}/shape.inl
		${SRCROOT}/rectangleShape.inl
		${SRCROOT}/modelLoader.inl    
		${SRCROOT}/particleSystemUpdater.inl
		${SRCROOT}/morphAnimUpdater.inl
		${SRCROOT}/boneAnimUpdater.inl
		${SRCROOT}/linkedListRenderer.inl
		${SRCROOT}/shadowRenderer.inl
		${SRCROOT}/envMapRenderer.inl
		${SRCROOT}/renderWindow.inl
		${SRCROOT}/mesh.inl
		${SRCROOT}/renderGraph.inl
	)
	target_compile_features(odfaeg-graphics-mod PUBLIC cxx_std_20)
	target_include_directories(odfaeg-graphics-mod
			PUBLIC
			$<INSTALL_INTERFACE:include>
			$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	)	
	target_include_directories(odfaeg-graphics-mod
			PUBLIC
			$<INSTALL_INTERFACE:include>
			$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/odfaeg/external>
	)	
	target_include_directories(odfaeg-graphics-mod
	INTERFACE
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
			$<BUILD_INTERFACE:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/>
			$<INSTALL_INTERFACE:include>
	)
	if (ODFAEG_COMPILER STREQUAL "msvc")
		find_package(Vulkan REQUIRED)
		find_package(assimp CONFIG REQUIRED)
		find_package(freetype CONFIG REQUIRED)
		find_package(unofficial-shaderc CONFIG REQUIRED)
	else()
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(VULKAN REQUIRED vulkan)
		pkg_check_modules(FREETYPE REQUIRED freetype2)
		pkg_check_modules(SHADERC REQUIRED shaderc)
		pkg_check_modules(ASSIMP REQUIRED assimp)
	endif()
	#target_include_directories(odfaeg-graphics-mod PRIVATE ${VCPKG_INSTALLED_DIR}/x64-windows/include)
	target_include_directories(odfaeg-graphics-mod PRIVATE ${VULKAN_INCLUDE_DIRS})
	#target_include_directories(odfaeg-graphics-mod PRIVATE $ENV{VULKAN_SDK}/Include)
	
	target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/vma)
	
	add_library(vma ${PROJECT_SOURCE_DIR}/external/vma/vk_mem_alloc.cpp)
	target_include_directories(vma PRIVATE  $ENV{VULKAN_SDK}/Include)
	target_link_libraries(odfaeg-graphics-mod PRIVATE vma)
	#set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
	
	
	
	target_link_libraries(odfaeg-graphics-mod PUBLIC odfaeg-window-mod odfaeg-entity-mod odfaeg-physics-mod odfaeg-math-mod odfaeg-core-mod PUBLIC ${VULKAN_LIBRARIES})

	
	target_include_directories(odfaeg-graphics-mod PRIVATE ${ASSIMP_INCLUDE_DIRS})
	target_link_directories(odfaeg-graphics-mod PRIVATE ${ASSIMP_LIBRARY_DIRS})
	target_link_libraries(odfaeg-graphics-mod PRIVATE ${ASSIMP_LIBRARIES}
	${SHADERC_LIBRARIES})
	target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/stb)
	target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external)
	target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/entt)
	target_include_directories(odfaeg-graphics-mod PRIVATE ${PROJECT_SOURCE_DIR}/external/gli)
	
	
	target_link_libraries(odfaeg-graphics PRIVATE odfaeg-graphics-mod)
	install(
			TARGETS odfaeg-graphics			
			EXPORT odfaeg-targets
			FILE_SET HEADERS DESTINATION include
	)
		install(
			TARGETS odfaeg-graphics-mod	
			vma			
			EXPORT odfaeg-targets
			FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Graphics
	)
	install(
			DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-graphics-mod.dir/
			DESTINATION share/odfaeg/pcm/Graphics
			FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
	)
else()	
	target_sources(odfaeg-graphics PRIVATE ${SPV_OUTPUTS})
	target_sources(odfaeg-graphics
		PUBLIC
			PRIVATE
			${SRCROOT}/graphics.hpp
			${SRCROOT}/debug.hpp
			${SRCROOT}/instance.hpp
			${SRCROOT}/device.hpp
			${SRCROOT}/commandPool.hpp
			${SRCROOT}/buffer.hpp
			${SRCROOT}/image.hpp		
			${SRCROOT}/blendMode.hpp		
			${SRCROOT}/imageLoader.hpp
			${SRCROOT}/texture.hpp
			${SRCROOT}/shader.hpp
			${SRCROOT}/renderStates.hpp               
			${SRCROOT}/material.hpp
			${SRCROOT}/vertexBuffer.hpp
			${SRCROOT}/descriptor.hpp
			${SRCROOT}/pipeline.hpp
			${SRCROOT}/renderPass.hpp
			${SRCROOT}/frameBuffer.hpp
			${SRCROOT}/fence.hpp
			${SRCROOT}/semaphore.hpp		
			${SRCROOT}/projMatrix.hpp
			${SRCROOT}/viewMatrix.hpp
			${SRCROOT}/viewportMatrix.hpp
			${SRCROOT}/camera.hpp
			${SRCROOT}/gpuContext.hpp
			${SRCROOT}/renderTarget.hpp
			${SRCROOT}/renderTexture.hpp
			${SRCROOT}/swapchain.hpp	
			${SRCROOT}/shape.hpp
			${SRCROOT}/rectangleShape.hpp
			${SRCROOT}/modelLoader.hpp		
			${SRCROOT}/particleSystemUpdater.hpp
			${SRCROOT}/morphAnimUpdater.hpp	
			${SRCROOT}/boneAnimUpdater.hpp
			${SRCROOT}/linkedListRenderer.hpp
			${SRCROOT}/envMapRenderer.hpp
			${SRCROOT}/shadowRenderer.hpp
			${SRCROOT}/renderWindow.hpp
			${SRCROOT}/mesh.hpp
			${SRCROOT}/iComponent.hpp
			${SRCROOT}/widget.hpp
			${SRCROOT}/iRenderer.hpp
			${SRCROOT}/renderGraph.hpp
			${SRCROOT}/componentManager.hpp
			${SRCROOT}/lightningRenderer.hpp
)
	target_sources(odfaeg-graphics PRIVATE
		${SRCROOT}/debug.inl
		${SRCROOT}/instance.inl
		${SRCROOT}/commandPool.inl
		${SRCROOT}/device.inl
		${SRCROOT}/buffer.inl
		${SRCROOT}/image.inl    
		${SRCROOT}/blendMode.inl
		${SRCROOT}/imageLoader.inl
		${SRCROOT}/texture.inl
		${SRCROOT}/shader.inl
		${SRCROOT}/renderStates.inl
		${SRCROOT}/material.inl
		${SRCROOT}/vertexBuffer.inl
		${SRCROOT}/descriptor.inl
		${SRCROOT}/pipeline.inl
		${SRCROOT}/renderPass.inl
		${SRCROOT}/frameBuffer.inl
		${SRCROOT}/fence.inl
		${SRCROOT}/semaphore.inl
		${SRCROOT}/projMatrix.inl
		${SRCROOT}/camera.inl
		${SRCROOT}/viewportMatrix.inl
		${SRCROOT}/viewMatrix.inl
		${SRCROOT}/gpuContext.inl
		${SRCROOT}/renderTarget.inl
		${SRCROOT}/renderTexture.inl
		${SRCROOT}/swapchain.inl 
		${SRCROOT}/shape.inl
		${SRCROOT}/rectangleShape.inl
		${SRCROOT}/modelLoader.inl    
		${SRCROOT}/particleSystemUpdater.inl
		${SRCROOT}/morphAnimUpdater.inl
		${SRCROOT}/boneAnimUpdater.inl
		${SRCROOT}/linkedListRenderer.inl
		${SRCROOT}/shadowRenderer.inl
		${SRCROOT}/envMapRenderer.inl
		${SRCROOT}/renderWindow.inl
		${SRCROOT}/mesh.inl
		${SRCROOT}/renderGraph.inl
		${SRCROOT}/lightningRenderer.inl
	)
	target_sources(odfaeg-graphics PRIVATE
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
		${SRCROOT}/renderGraph.cpp
		${SRCROOT}/lightningRenderer.cpp
	)
	target_include_directories(odfaeg-graphics PRIVATE
		${PROJECT_SOURCE_DIR}/external/meshoptimizer/src
	)
	target_include_directories(odfaeg-graphics PRIVATE ${PROJECT_SOURCE_DIR}/external/vma)
	#target_include_directories(odfaeg-graphics-mod PRIVATE ${VCPKG_INSTALLED_DIR}/x64-windows/include)
	#target_include_directories(odfaeg-graphics-mod PRIVATE ${Vulkan_INCLUDE_DIRS})
	add_library(vma ${PROJECT_SOURCE_DIR}/external/vma/vk_mem_alloc.cpp)
	target_link_libraries(odfaeg-graphics PRIVATE vma)
	#set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(VULKAN REQUIRED vulkan)
	pkg_check_modules(FREETYPE REQUIRED freetype2)
	pkg_check_modules(SHADERC REQUIRED shaderc)
	pkg_check_modules(ASSIMP REQUIRED assimp)
	#find_package(Vulkan REQUIRED)
	target_include_directories(odfaeg-graphics PRIVATE ${VULKAN_INCLUDE_DIRS})
	target_link_libraries(odfaeg-graphics PUBLIC odfaeg-window odfaeg-entity odfaeg-physics odfaeg-math odfaeg-core PUBLIC ${VULKAN_LIBRARIES})

	#find_package(assimp CONFIG REQUIRED)
	#find_package(freetype CONFIG REQUIRED)
	#find_package(shaderc CONFIG REQUIRED)
	target_include_directories(odfaeg-graphics PRIVATE ${ASSIMP_INCLUDE_DIRS})
	target_link_directories(odfaeg-graphics PRIVATE ${ASSIMP_LIBRARY_DIRS})
	target_link_libraries(odfaeg-graphics PRIVATE ${ASSIMP_LIBRARIES}
	${SHADERC_LIBRARIES})
	target_include_directories(odfaeg-graphics PRIVATE ${PROJECT_SOURCE_DIR}/external/stb)
	target_include_directories(odfaeg-graphics PRIVATE ${PROJECT_SOURCE_DIR}/external)
	target_include_directories(odfaeg-graphics PRIVATE ${PROJECT_SOURCE_DIR}/external/entt)
	target_include_directories(odfaeg-graphics PRIVATE ${PROJECT_SOURCE_DIR}/external/gli)
	target_include_directories(odfaeg-graphics
			PUBLIC
			$<INSTALL_INTERFACE:include>
			$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	)	
	target_include_directories(odfaeg-graphics
    PUBLIC
        $<INSTALL_INTERFACE:include/odfaeg/external/vma>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/vma>
    )
	target_include_directories(odfaeg-graphics
    PUBLIC
        $<INSTALL_INTERFACE:include/odfaeg/external/gli>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/gli>
    )
	target_include_directories(odfaeg-graphics
    PUBLIC
        $<INSTALL_INTERFACE:include/odfaeg/external/stb>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/stb>
    )
	target_include_directories(odfaeg-graphics
    PUBLIC
        $<INSTALL_INTERFACE:include/odfaeg/external/entt>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/external/entt>
    )
	# Installer la lib statique
	install(
		TARGETS odfaeg-graphics
		EXPORT odfaeg-targets		
		ARCHIVE DESTINATION lib
		FILE_SET HEADERS DESTINATION include/odfaeg/Graphics
	)
	install(
        TARGETS vma
        EXPORT odfaeg-targets
        ARCHIVE DESTINATION lib
    )
	# Installer les headers publics
	install(
		DIRECTORY ${SRCROOT}/
		DESTINATION include/odfaeg/Graphics
		FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl"
	)
endif()


