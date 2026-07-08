
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Window)
set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Window)

#file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "${SRCROOT}/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${INCROOT}/*.hpp")

add_library(odfaeg-window-mod STATIC)
#target_include_directories(odfaeg-window-mod PRIVATE ${VCPKG_INSTALLED_DIR}/x64-windows/include)
#target_include_directories(odfaeg-window-mod PRIVATE ${Vulkan_INCLUDE_DIRS})
if (UNIX)
target_sources(odfaeg-window-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES                 
        FILES 
        ${SRCROOT}/iCursorType.ixx	
	${SRCROOT}/iEvent.ixx	
        ${SRCROOT}/iKeyboard.ixx
        ${SRCROOT}/Linux/display.ixx
	${SRCROOT}/Linux/x11Keyboard.ixx        
	${SRCROOT}/windowStyle.ixx
        ${SRCROOT}/videoMode.ixx
        ${SRCROOT}/Linux/x11VideoMode.ixx
	${SRCROOT}/videoModeImpl.ixx	
	${SRCROOT}/Linux/x11Cursor.ixx
	${SRCROOT}/cursorImpl.ixx
	${SRCROOT}/cursor.ixx  
	${SRCROOT}/iMouse.ixx
        ${SRCROOT}/Linux/x11Mouse.ixx
        ${SRCROOT}/Linux/x11Window.ixx
	${SRCROOT}/windowImpl.ixx
	${SRCROOT}/window.ixx
	${SRCROOT}/action.ixx
	${SRCROOT}/command.ixx
	${SRCROOT}/listener.ixx
)
target_sources(odfaeg-window-mod
    PRIVATE     
        ${SRCROOT}/iKeyboard.cpp
        ${SRCROOT}/Linux/display.cpp
	${SRCROOT}/Linux/x11Keyboard.cpp
        ${SRCROOT}/videoMode.cpp
        ${SRCROOT}/Linux/x11VideoMode.cpp
	${SRCROOT}/videoModeImpl.cpp
	${SRCROOT}/Linux/x11Cursor.cpp
	${SRCROOT}/cursorImpl.cpp
	${SRCROOT}/cursor.cpp
	${SRCROOT}/iMouse.cpp
        ${SRCROOT}/Linux/x11Mouse.cpp
        ${SRCROOT}/Linux/x11Window.cpp
	${SRCROOT}/windowImpl.cpp
	${SRCROOT}/window.cpp
	${SRCROOT}/action.cpp
	${SRCROOT}/command.cpp
)
target_link_libraries(odfaeg-window PRIVATE
    X11
    Xrandr
    Xrender
    Xext
    Xfixes
    Xcursor
    Xi
    Xinerama
)
elseif(WIN32)
target_sources(odfaeg-window-mod
    PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES                 
        FILES         
        ${SRCROOT}/iCursorType.ixx	
	${SRCROOT}/iEvent.ixx	
        ${SRCROOT}/iKeyboard.ixx        
	${SRCROOT}/Windows/win32Keyboard.ixx        
	${SRCROOT}/windowStyle.ixx
        ${SRCROOT}/videoMode.ixx
        ${SRCROOT}/Windows/win32VideoMode.ixx
	${SRCROOT}/videoModeImpl.ixx	
	${SRCROOT}/Windows/win32Cursor.ixx
	${SRCROOT}/cursorImpl.ixx
	${SRCROOT}/cursor.ixx 
        ${SRCROOT}/Windows/win32Mouse.ixx
        ${SRCROOT}/Windows/win32Window.ixx
	${SRCROOT}/windowImpl.ixx
	${SRCROOT}/window.ixx	
	${SRCROOT}/iMouse.ixx
	${SRCROOT}/action.ixx
	${SRCROOT}/command.ixx
	${SRCROOT}/listener.ixx	
)
target_sources(odfaeg-window-mod
    PRIVATE           
        ${SRCROOT}/iKeyboard.cpp
	${SRCROOT}/Windows/win32Keyboard.cpp
        ${SRCROOT}/videoMode.cpp
        ${SRCROOT}/Windows/win32VideoMode.cpp
	${SRCROOT}/videoModeImpl.cpp
	${SRCROOT}/Windows/win32Cursor.cpp
	${SRCROOT}/cursorImpl.cpp
	${SRCROOT}/cursor.cpp
	${SRCROOT}/iMouse.cpp
        ${SRCROOT}/Windows/win32Mouse.cpp
        ${SRCROOT}/Windows/win32Window.cpp
	${SRCROOT}/windowImpl.cpp
	${SRCROOT}/window.cpp
	${SRCROOT}/action.cpp
	${SRCROOT}/command.cpp
)
endif()
target_compile_features(odfaeg-window-mod PUBLIC cxx_std_20)
target_link_libraries(odfaeg-window-mod PUBLIC odfaeg-physics-mod odfaeg-entity-mod odfaeg-math-mod odfaeg-core-mod)

add_library(odfaeg-window STATIC)

target_sources(odfaeg-window
    PUBLIC
        FILE_SET HEADERS TYPE HEADERS
        BASE_DIRS ${INCROOT}
        FILES ${HEADERS}
)
target_sources(odfaeg-window PRIVATE ${SRC})
#target_include_directories(odfaeg-window-mod
#		PUBLIC
#		${CMAKE_SOURCE_DIR}/include>
#)

target_include_directories(odfaeg-window-mod
		PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)
target_include_directories(odfaeg-window-mod PRIVATE ${PROJECT_SOURCE_DIR}/external)
target_include_directories(odfaeg-base
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<BUILD_INTERFACE:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/>
        $<INSTALL_INTERFACE:include>
)
#set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
find_package(PkgConfig REQUIRED MODULE)
#find_package(Vulkan REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)
target_include_directories(odfaeg-window-mod PUBLIC ${VULKAN_INCLUDE_DIRS})
#link_directories(${VULKAN_LIBRARY_DIRS})
target_link_libraries(odfaeg-window PRIVATE odfaeg-window-mod PRIVATE ${VULKAN_LIBRAIRIES})
install(
    TARGETS odfaeg-window
    EXPORT odfaeg-targets
    FILE_SET HEADERS DESTINATION include
)
install(
		TARGETS odfaeg-window-mod
		EXPORT odfaeg-targets
		FILE_SET cxx_modules DESTINATION share/odfaeg/modules/Window
)
install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/odfaeg-window-mod.dir/
    DESTINATION share/odfaeg/pcm/Window
    FILES_MATCHING PATTERN "*.pcm" PATTERN "*.modmap"
)




