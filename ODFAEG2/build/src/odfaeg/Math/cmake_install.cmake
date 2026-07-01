# Install script for directory: /home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Math/libodfaeg-math.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Math/export.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Math/libodfaeg-math-mod.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/modules/Math" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/computer.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/distribution.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/maths.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/matrix.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/plane.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/quaternion.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/ray.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/triangle.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Math/vec.ixx"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/pcm/Math" TYPE DIRECTORY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Math/CMakeFiles/odfaeg-math-mod.dir/" FILES_MATCHING REGEX "/[^/]*\\.pcm$" REGEX "/[^/]*\\.modmap$")
endif()

