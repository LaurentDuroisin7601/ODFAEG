include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)


set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Window)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Window)

set(SRC_WINDOW
	${INCROOT}/action.h	
	${SRCROOT}/actions.cpp
	${INCROOT}/command.h
	${SRCROOT}/command.cpp
	${INCROOT}/listener.h
	${SRCROOT}/listener.cpp
	${INCROOT}/contextImpl.hpp
	${SRCROOT}/contextImpl.cpp	
	${INCROOT}/contextSettings.hpp
	${SRCROOT}/contextSettings.cpp
	${INCROOT}/iKeyboard.hpp
	${SRCROOT}/iKeyboard.cpp
	${INCROOT}/iMouse.hpp
	${SRCROOT}/iMouse.cpp
	${INCROOT}/windowImpl.hpp
	${SRCROOT}/windowImpl.cpp
	${INCROOT}/iWindow.hpp
	${INCROOT}/iContext.hpp
	${INCROOT}/windowFactory.hpp
	${SRCROOT}/windowFactory.cpp
	${INCROOT}/contextFactory.hpp
	${SRCROOT}/contextFactory.cpp
	${INCROOT}/window.hpp
	${SRCROOT}/window.cpp
	${INCROOT}/context.hpp
	${SRCROOT}/context.cpp	
	${INCROOT}/iGlResource.hpp
	${INCROOT}/glResource.hpp
	${SRCROOT}/glResource.cpp	
	${INCROOT}/SFML/sfmlWindowImpl.hpp
	${SRCROOT}/SFML/sfmlWindowImpl.cpp
	${INCROOT}/SFML/sfmlKeyboard.hpp
	${SRCROOT}/SFML/sfmlKeyboard.cpp
	${INCROOT}/SFML/sfmlMouse.hpp
	${SRCROOT}/SFML/sfmlMouse.cpp
	${INCROOT}/SFML/sfmlContextImpl.hpp
	${SRCROOT}/SFML/sfmlContextImpl.cpp
	${INCROOT}/GLFW/vkGLFWWindow.hpp
	${SRCROOT}/GLFW/vkGLFWWindow.cpp
	${INCROOT}/vkDebug.hpp
	${SRCROOT}/vkDebug.cpp
	${INCROOT}/vkSettup.hpp
	${SRCROOT}/vkSettup.cpp
)
if (UNIX)
set(PLATEFORM_SRC
${INCROOT}/Linux/x11Window.hpp
${SRCROOT}/Linux/x11Window.cpp
${INCROOT}/Linux/glxContext.hpp
${SRCROOT}/Linux/glxContext.cpp	
${INCROOT}/Linux/display.hpp
${SRCROOT}/Linux/display.cpp
${INCROOT}/Linux/x11Keyboard.hpp
${SRCROOT}/Linux/x11Keyboard.cpp
${INCROOT}/Linux/x11Mouse.hpp
${SRCROOT}/Linux/x11Mouse.cpp
${SRCROOT}/Linux/glxExtensions.hpp
${SRCROOT}/Linux/glxExtensions.cpp	
)
elseif(WIN32)
set(PLATEFORM_SRC
${INCROOT}/Windows/win32Window.hpp
${SRCROOT}/windows/win32Window.cpp
${INCROOT}/Windows/wglContext.hpp
${SRCROOT}/Windows/wglContext.cpp	
${INCROOT}/Windows/wglExtensions.hpp
${SRCROOT}/Windows/wglExtensions.cpp
${SRCROOT}/Windows/win32Keyboard.cpp
${INCROOT}/Windows/w32Keyboard.hpp
${INCROOT}/Windows/win32Mouse.hpp
${SRCROOT}/Windows/win32Mouse.cpp
)
endif()
include_directories(${CMAKE_INCLUDE_PATH} ${FREETYPE_INCLUDE_DIRS})
link_directories(${CMAKE_LIBRARY_PATH})

sfgl_add_library(odfaeg-window
                 SOURCES ${SRC_WINDOW} ${PLATEFORM_SRC}
				 DEPENDS odfaeg-core odfaeg-math)
target_link_libraries (odfaeg-window ${SFML_LIBRARIES})

	
	
