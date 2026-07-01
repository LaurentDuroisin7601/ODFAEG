# Install script for directory: /home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Core/libodfaeg-core.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Core/export.hpp"
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Core/macros.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Core/libodfaeg-core-mod.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/modules/Core" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/any.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/archive.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/class.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/clock.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/delegate.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/factory.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/inputStream.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/metaprog.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/nonCopyable.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/resourceCache.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/resourceManager.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/runtime_compiler.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/serialization.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/stateManager.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/string.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/threadPool.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/timer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/utf.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/utilities.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/variant.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Core/worker.ixx"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/pcm/Core" TYPE DIRECTORY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Core/CMakeFiles/odfaeg-core-mod.dir/" FILES_MATCHING REGEX "/[^/]*\\.pcm$" REGEX "/[^/]*\\.modmap$")
endif()

