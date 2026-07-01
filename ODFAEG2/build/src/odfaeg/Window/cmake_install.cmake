# Install script for directory: /home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Window/libodfaeg-window.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Window/export.hpp"
    "/home/laurent/ODFAEG-master/ODFAEG2/include/odfaeg/Window/windowHandle.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Window/libodfaeg-window-mod.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/modules/Window" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/iCursorType.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/iEvent.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/iKeyboard.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/windowStyle.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/videoMode.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/videoModeImpl.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/cursorImpl.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/cursor.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/iMouse.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/windowImpl.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/window.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/action.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/command.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/listener.ixx"
    )
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/modules/Window/Linux" TYPE FILE FILES
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/display.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/x11Keyboard.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/x11VideoMode.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/x11Cursor.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/x11Mouse.ixx"
    "/home/laurent/ODFAEG-master/ODFAEG2/src/odfaeg/Window/Linux/x11Window.ixx"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/odfaeg/pcm/Window" TYPE DIRECTORY FILES "/home/laurent/ODFAEG-master/ODFAEG2/build/src/odfaeg/Window/CMakeFiles/odfaeg-window-mod.dir/" FILES_MATCHING REGEX "/[^/]*\\.pcm$" REGEX "/[^/]*\\.modmap$")
endif()

