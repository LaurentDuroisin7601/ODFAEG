include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Window)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Window)

set(SRC_WINDOW
	${INCROOT}/action.h
	${SRCROOT}/actions.cpp
	${INCROOT}/command.h
	${INCROOT}/command.tpp
	${INCROOT}/listener.h
	${SRCROOT}/command.cpp
	${INCROOT}/context.hpp
	${INCROOT}/contextFactory.hpp
	${INCROOT}/contextImpl.hpp
	${INCROOT}/contextSettings.hpp
	${INCROOT}/glResource.hpp
	${INCROOT}/iContext.hpp
	${INCROOT}/iEvent.hpp
	${INCROOT}/iGlResource.hpp
	${INCROOT}/iKeyboard.hpp
	${INCROOT}/iMouse.hpp
	${INCROOT}/iWindow.hpp
	${INCROOT}/keyboard.hpp
	${INCROOT}/window.hpp
	${INCROOT}/windowFactory.hpp
	${INCROOT}/windowImpl.hpp
	${INCROOT}/windowStyle.hpp
	${SRCROOT}/context.cpp
	${SRCROOT}/contextFactory.cpp
	${SRCROOT}/contextImpl.cpp
	${SRCROOT}/contextSettings.cpp
	${SRCROOT}/glResource.cpp
	${SRCROOT}/iKeyboard.cpp
	${SRCROOT}/iMouse.cpp
	${SRCROOT}/window.cpp
	${SRCROOT}/windowFactory.cpp
	${SRCROOT}/windowImpl.cpp
	${INCROOT}/SFML/sfmlContextImpl.hpp
	${INCROOT}/SFML/sfmlKeyboard.hpp
	${INCROOT}/SFML/sfmlMouse.hpp
	${INCROOT}/SFML/sfmlWindowImpl.hpp
	${SRCROOT}/SFML/sfmlContextImpl.cpp
	${SRCROOT}/SFML/sfmlKeyboard.cpp
	${SRCROOT}/SFML/sfmlMouse.cpp
	${SRCROOT}/SFML/sfmlWindowImpl.cpp
	
		${INCROOT}/Windows/w32Keyboard.hpp
		${INCROOT}/Windows/wglContext.hpp
		${INCROOT}/Windows/win32Mouse.hpp
		${INCROOT}/Windows/win32Window.hpp
		${SRCROOT}/Windows/wglContext.cpp
		${SRCROOT}/Windows/wglExtensions.cpp
		${INCROOT}/Windows/wglExtensions.hpp
		${SRCROOT}/Windows/win32Keyboard.cpp
		${SRCROOT}/Windows/win32Mouse.cpp
		${SRCROOT}/Windows/win32Window.cpp
		
		${INCROOT}/vkDebug.hpp
		${SRCROOT}/vkDebug.cpp
		${INCROOT}/vkSettup.hpp
		${SRCROOT}/vkSettup.cpp
		${INCROOT}/GLFW/vkGLFWWindow.hpp
		${SRCROOT}/GLFW/vkGLFWWindow.cpp
	)
find_package (SFML 2 REQUIRED)
include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})
sfgl_add_library(odfaeg-window
		 SOURCES ${SRC_WINDOW} 
		 DEPENDS odfaeg-core odfaeg-math)
target_link_libraries(odfaeg-window ${SFML_LIBRARIES})