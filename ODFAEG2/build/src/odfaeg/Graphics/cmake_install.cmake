# Install script for directory: /home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RELEASE")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Graphics/libodfaeg-graphics.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Graphics/export.hpp"
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Graphics/windowHandle.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Graphics/libodfaeg-graphics-mod.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/modules/Graphics" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/primitiveType.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/color.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/debug.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/instance.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/device.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/buffer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/image.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/commandPool.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/blendMode.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/rect.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/imageLoader.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/texture.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/shader.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/renderStates.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/transformable.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/vertex.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/material.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/vertexBuffer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/descriptor.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/pipeline.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/renderPass.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/frameBuffer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/fence.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/semaphore.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/gameObject.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/projMatrix.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/viewMatrix.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/viewportMatrix.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/camera.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/renderTarget.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/renderTexture.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/swapchain.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/renderWindow.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/shape.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/rectangleShape.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/tile.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/model.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/modelLoader.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/assimpHelper.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/gpuContext.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/particle.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/emittors.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/particleSystem.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/particleSystemUpdater.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/morphAnim.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/morphAnimUpdater.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/bone.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/animation.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/animator.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/boneAnimUpdater.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/linkedListRenderer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/shadowRenderer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/cube.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Graphics/plane.ixx"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Graphics/libvma.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Graphics/libmeshoptimizer.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/pcm/Graphics" TYPE DIRECTORY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Graphics/CMakeFiles/odfaeg-graphics-mod.dir/" FILES_MATCHING REGEX "/[^/]*\\.pcm$" REGEX "/[^/]*\\.modmap$")
endif()

