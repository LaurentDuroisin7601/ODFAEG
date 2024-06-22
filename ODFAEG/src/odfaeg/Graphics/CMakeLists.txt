include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Graphics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Graphics)

set(SRC_GRAPHICS
	${INCROOT}/application.h
	${SRCROOT}/application.cpp	
	${INCROOT}/lightRenderComponent.hpp
	${SRCROOT}/lightRenderComponent.cpp
	${INCROOT}/baseChangementMatrix.h
	${SRCROOT}/baseChangementMatrix.cpp		
	${INCROOT}/export.hpp	
	${INCROOT}/viewMatrix.h
	${SRCROOT}/viewMatrix.cpp
	${INCROOT}/projMatrix.h
	${SRCROOT}/projMatrix.cpp
	${INCROOT}/entityManager.h	
	${INCROOT}/viewportMatrix.h
	${SRCROOT}/viewport.cpp
	${INCROOT}/renderTexture.h
	${SRCROOT}/renderTexture.cpp
	${SRCROOT}/renderTextureImpl.h
	${SRCROOT}/renderTextureImplDefault.h
	${SRCROOT}/renderTextureImplFBO.h
	${SRCROOT}/renderTextureImpl.cpp
	${SRCROOT}/renderTextureImplDefault.cpp
	${SRCROOT}/renderTextureImplFBO.cpp
	${SRCROOT}/textureSaver.h
	${SRCROOT}/textureSaver.cpp
	${SRCROOT}/glCheck.h
	${SRCROOT}/glCheck.cpp	
	${INCROOT}/shader.h
	${SRCROOT}/shader.cpp
	${INCROOT}/texture.h
	${SRCROOT}/texture.cpp
	${INCROOT}/shape.h
	${SRCROOT}/shape.cpp
	${INCROOT}/convexShape.h
	${SRCROOT}/convexShape.cpp
	${INCROOT}/rectangleShape.h
	${SRCROOT}/rectangleShape.cpp
	${INCROOT}/circleShape.h
	${SRCROOT}/circleShape.cpp
	${INCROOT}/font.h
	${SRCROOT}/font.cpp
	${INCROOT}/text.h
        ${SRCROOT}/text.cpp
	${INCROOT}/drawable.h	
	${SRCROOT}/drawable.cpp
	${INCROOT}/transformable.h	
	${INCROOT}/renderStates.h
	${SRCROOT}/renderStates.cpp
	${INCROOT}/renderTarget.h
	${SRCROOT}/renderTarget.cpp
	${INCROOT}/renderWindow.h
	${SRCROOT}/renderWindow.cpp	
	${INCROOT}/vertex.h
	${SRCROOT}/vertex.cpp
	${INCROOT}/vertexArray.h
	${SRCROOT}/vertexArray.cpp
	${INCROOT}/vertexBuffer.hpp
	${SRCROOT}/vertexBuffer.cpp
	${INCROOT}/view.h
	${SRCROOT}/view.cpp
	${INCROOT}/component.h
	${SRCROOT}/component.cpp
	${INCROOT}/renderComponentManager.h
	${SRCROOT}/renderComponentManager.cpp
	${INCROOT}/vbo.h
	${SRCROOT}/vbo.cpp
	${INCROOT}/batcher.h
	${SRCROOT}/batcher.cpp
	${INCROOT}/cellMap.h
	${SRCROOT}/cellMap.cpp	
	${INCROOT}/sprite.h
	${SRCROOT}/sprite.cpp
	${INCROOT}/entity.h
	${SRCROOT}/entity.cpp
	${SRCROOT}/GlDebug.hpp
	${SRCROOT}/GlDebug.cpp
	${SRCROOT}/animation.cpp	
	${INCROOT}/tile.h
	${SRCROOT}/tile.cpp	
	${INCROOT}/shadowRenderComponent.hpp
	${SRCROOT}/shadowRenderCompoenent.cpp
	${INCROOT}/reflectRefractRenderComponent.hpp
	${SRCROOT}/reflectRefractRenderComponent.cpp
	${INCROOT}/raytracingRenderComponent.hpp
	${SRCROOT}/raytracingRenderComponent.cpp
	${INCROOT}/animatedEntity.h	
	${INCROOT}/model.h
	${SRCROOT}/model.cpp
	${INCROOT}/selectable.h
	${INCROOT}/light.h	
	${INCROOT}/anim.h
	${INCROOT}/gridMap.h
	${SRCROOT}/gridMap.cpp	
	${INCROOT}/map.h
	${SRCROOT}/map.cpp
	${INCROOT}/tGround.h
	${SRCROOT}/tGround.cpp
	${INCROOT}/billBoard.h
	${SRCROOT}/billBoard.cpp	
	${INCROOT}/world.h
	${SRCROOT}/world.cpp
	${INCROOT}/glExtensions.hpp
	${SRCROOT}/glExtensions.cpp	
	${INCROOT}/glLoader.hpp
	${SRCROOT}/glLoader.cpp
	${INCROOT}/mesh.hpp
	${SRCROOT}/mesh.cpp
	${INCROOT}/boneAnimation.hpp
	${SRCROOT}/boneAnimation.cpp
	${INCROOT}/lightComponent.h
	${INCROOT}/heavyComponent.h
	${SRCROOT}/heavyComponent.cpp
	${INCROOT}/entityManager.h	
	${INCROOT}/perPixelLinkedListRenderComponent.hpp
	${SRCROOT}/perPixelLinkedListRenderComponent.cpp
	${INCROOT}/gameObject.hpp
	${SRCROOT}/gameObject.cpp
	${INCROOT}/boneAnimation.hpp
	${SRCROOT}/boneAnimation.cpp
	${INCROOT}/animatedEntity.h
	${SRCROOT}/animatedEntity.cpp
	)
source_group ("graphics" FILES ${SRC_GRAPHICS})

set(SRC_ECS
	${INCROOT}/ECS/cell.hpp
	${SRCROOT}/ECS/cell.cpp
	${INCROOT}/ECS/grid.hpp
	${SRCROOT}/ECS/grid.cpp
	${INCROOT}/ECS/modelFactory.hpp
	${INCROOT}/ECS/systems.hpp
	${INCROOT}/ECS/components.hpp
	${SRCROOT}/ECS/component.cpp
	${INCROOT}/ECS/world.hpp
	${SRCROOT}/ECS/system.cpp
	${INCROOT}/ECS/scene.hpp
	${SRCROOT}/ECS/scene.cpp
	${INCROOT}/ECS/heavyComponent.hpp
	${INCROOT}/ECS/component.hpp
	${INCROOT}/ECS/perPixelLinkedList.hpp
	${SRCROOT}/ECS/perPixelLinkedList.cpp
	${INCROOT}/ECS/entitiesUpdater.hpp
	${INCROOT}/ECS/application.hpp
	${SRCROOT}/ECS/application.cpp
	${INCROOT}/ECS/animationUpdater.hpp
	${SRCROOT}/ECS/animationUpdater.cpp	
	)
	
set(SRC_GRAPHICS_2D
	
	${INCROOT}/2D/wall.h
	${SRCROOT}/2D/wall.cpp	
	${INCROOT}/2D/ambientLight.h
	${SRCROOT}/2D/ambientLight.cpp
	${INCROOT}/2D/ponctualLight.h
	${SRCROOT}/2D/ponctualLight.cpp
	${INCROOT}/2D/directionnalLight.h
	${SRCROOT}/2D/directionnalLight.cpp
	${INCROOT}/2D/decor.h
	${SRCROOT}/2D/decor.cpp)
	
source_group ("2d_graphics" FILES ${SRC_GRAPHICS_2D})

set(SRC_GRAPHICS_3D	
	${INCROOT}/3D/cube.h
	${SRCROOT}/3D/cube.cpp
	${INCROOT}/3D/model.hpp
	${SRCROOT}/3D/model.cpp)
	
source_group("3d_graphics" FILES ${SRC_GRAPHICS_3D})
set(SRC_GUI
	${INCROOT}/GUI/tabPane.hpp
	${SRCROOT}/GUI/tabPane.cpp
	${INCROOT}/GUI/node.hpp
	${SRCROOT}/GUI/node.cpp
	${INCROOT}/GUI/table.hpp
	${SRCROOT}/GUI/table.cpp
	${INCROOT}/GUI/icon.hpp
	${SRCROOT}/GUI/icon.cpp
	${INCROOT}/GUI/button.hpp
	${SRCROOT}/GUI/button.cpp
	${INCROOT}/GUI/textArea.hpp
	${SRCROOT}/GUI/textArea.cpp
	${INCROOT}/GUI/optionPane.hpp
	${SRCROOT}/GUI/optionPane.cpp
	${INCROOT}/GUI/menubar.hpp
	${SRCROOT}/GUI/menubar.cpp
	${INCROOT}/GUI/menu.hpp
	${SRCROOT}/GUI/menu.cpp
	${INCROOT}/GUI/menuItem.hpp
	${SRCROOT}/GUI/menuItem.cpp
	${INCROOT}/GUI/filedialog.hpp
	${SRCROOT}/GUI/filedialog.cpp
	${INCROOT}/GUI/label.hpp
	${SRCROOT}/GUI/label.cpp
	${INCROOT}/GUI/panel.hpp
	${SRCROOT}/GUI/panel.cpp
	${INCROOT}/GUI/dropDownList.hpp
	${SRCROOT}/GUI/dropDownList.cpp
	${INCROOT}/GUI/passwordField.hpp
	${SRCROOT}/GUI/passwordField.cpp
	${INCROOT}/GUI/progressBar.hpp
	${SRCROOT}/GUI/progressBar.cpp
	${INCROOT}/GUI/checkBox.hpp
	${SRCROOT}/GUI/checkBox.cpp)
source_group ("gui" FILES ${SRC_GUI})

set(SRC_VULKAN
	${INCROOT}/Vulkan/instance.hpp
	${SRCROOT}/Vulkan/instance.cpp

	${INCROOT}/Vulkan/device.hpp
	${SRCROOT}/Vulkan/device.cpp

	${INCROOT}/Vulkan/vulkan_window.hpp
	${SRCROOT}/Vulkan/vulkan_window.cpp

	${INCROOT}/Vulkan/swapchain.hpp
	${SRCROOT}/Vulkan/swapchain.cpp
)
source_group("Vulkan" FILES ${SRC_VULKAN})

find_package (SFML 2 COMPONENTS window system graphics audio REQUIRED)
find_package(Vulkan REQUIRED)

find_package(Freetype REQUIRED)
include_directories(${CMAKE_INCLUDE_PATH} ${FREETYPE_INCLUDE_DIRS})
link_directories(${CMAKE_LIBRARY_PATH})

sfgl_add_library(odfaeg-graphics
				 SOURCES ${SRC_GRAPHICS} ${SRC_GRAPHICS_2D} ${SRC_GRAPHICS_3D} ${SRC_GUI} ${SRC_ECS}
				 DEPENDS odfaeg-core odfaeg-math odfaeg-physics)

target_link_libraries (odfaeg-graphics ${SFML_LIBRARIES})


	
	
