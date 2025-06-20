
cmake_minimum_required(VERSION 3.20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fdiagnostics-color)
    endif ()
endif ()

# define a macro that helps defining an option
macro(ODFAEG_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

# set a default build type if none was provided
# this has to be done before the project() instruction!
ODFAEG_set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")
set(CMAKE_CXX_STANDARD 20)
if (android_arm64)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()
add_definitions(-DGLEW_STATIC)
# project name
project(ODFAEG)

# include the configuration file
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)

# setup version numbers
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
#enable c++11 option and some optimisations options for size and speed.
#set(CMAKE_CXX_FLAGS "-std=c++20")
#set(CMAKE_CXX_FLAGS "-Winit-self")

# add the ODFAEG header path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# add an option for choosing the build type (shared or static)
ODFAEG_set_option(BUILD_SHARED_LIBS TRUE BOOL "TRUE to build ODFAEG as shared libraries, FALSE to build it as static libraries")

# add an option for building the examples
ODFAEG_set_option(ODFAEG_BUILD_EXAMPLES FALSE BOOL "TRUE to build the ODFAEG examples, FALSE to ignore them")

# add an option for building the API documentation
ODFAEG_set_option(ODFAEG_BUILD_DOC FALSE BOOL "TRUE to generate the API documentation, FALSE to ignore it")
if (WINDOWS OR LINUX)
ODFAEG_set_option(CMAKE_ARCHITECTURE i386 STRING "i386 for 32 bits compilation")
endif()
if(MACOSX)
    # add an option to build frameworks instead of dylibs (release only)
    ODFAEG_set_option(ODFAEG_BUILD_FRAMEWORKS FALSE BOOL "TRUE to build ODFAEG as frameworks libraries (release only), FALSE to build according to BUILD_SHARED_LIBS")
    
    # add an option to let the user specify a custom directory for frameworks installation (ODFAEG, sndfile, ...)
    ODFAEG_set_option(CMAKE_INSTALL_FRAMEWORK_PREFIX "/Library/Frameworks" STRING "Frameworks installation directory")
	#add an option to choose osx architectures. (32 or 64 bits)
	ODFAEG_set_option(CMAKE_OSX_ARCHITECTURES i386 STRING "i386 for 32 bits architectures")
   
endif()

# define ODFAEG_STATIC if the build type is not set to 'shared'
if(NOT BUILD_SHARED_LIBS)
    add_definitions(-DODFAEG_STATIC)
endif()

# remove SL security warnings with Visual C++
if(COMPILER_MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

# define an option for choosing between static and dynamic C runtime (Windows only)
if(WINDOWS)	
    ODFAEG_set_option(ODFAEG_USE_STATIC_STD_LIBS FALSE BOOL "TRUE to statically link to the standard libraries, FALSE to use them as DLLs")

    # the following combination of flags is not valid
    if (BUILD_SHARED_LIBS AND ODFAEG_USE_STATIC_STD_LIBS)
        message(FATAL_ERROR "BUILD_SHARED_LIBS and ODFAEG_USE_STATIC_STD_LIBS cannot be used together")
    endif()
	if (CMAKE_ARCHITECTURE STREQUAL "i386")
		add_definitions(-m32)
	endif()
    # for VC++, we can apply it globally by modifying the compiler flags
    if(COMPILER_MSVC AND ODFAEG_USE_STATIC_STD_LIBS)
        foreach(flag
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(${flag} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
        endforeach()
    endif()
endif()

# disable the rpath stuff
set(CMAKE_SKIP_BUILD_RPATH TRUE)
#set(SFML_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/headers/SFML/)
# setup Mac OS X stuff
if(MACOSX)
    # ODFAEG_BUILD_FRAMEWORKS needs two things :
    # first, it's available only for release
    #    (because cmake currently doesn't allow specifying a custom framework name so XXX-d is not possible)
    # secondly, it works only with BUILD_SHARED_LIBS enabled	
    if(ODFAEG_BUILD_FRAMEWORKS)
        # requirement #1
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
            message(FATAL_ERROR "CMAKE_BUILD_TYPE should be \"Release\" when ODFAEG_BUILD_FRAMEWORKS is TRUE")
            return()
        endif()

        # requirement #2
        if(NOT BUILD_SHARED_LIBS)
            message(FATAL_ERROR "BUILD_SHARED_LIBS should be TRUE when ODFAEG_BUILD_FRAMEWORKS is TRUE")
            return()
        endif()
    endif()
endif()

if(LINUX)
	if (CMAKE_ARCHITECTURE STREQUAL "i386")
		# add_definitions(-m32)
	endif()
    if(BUILD_SHARED_LIBS)		
		#[[
        ODFAEG_set_option(ODFAEG_INSTALL_PKGCONFIG_FILES TRUE BOOL "TRUE to automatically install pkg-config files so other projects can find ODFAEG")
        if(ODFAEG_INSTALL_PKGCONFIG_FILES)
            foreach(ODFAEG_module IN ITEMS all)
                CONFIGURE_FILE(
                    "tools/pkg-config/ODFAEG-${ODFAEG_module}.pc.in"
                    "tools/pkg-config/ODFAEG-${ODFAEG_module}.pc"
                    @ONLY)
	    INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/tools/pkg-config/ODFAEG-${ODFAEG_module}.pc"
                    DESTINATION "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig")
            endforeach()
        endif()
		]]
    else()
        if(ODFAEG_INSTALL_PKGCONFIG_FILES)
            message(WARNING "No pkg-config files are provided for the static ODFAEG libraries (ODFAEG_INSTALL_PKGCONFIG_FILES will be ignored).")
        endif()
    endif()
endif()

# enable project folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# add the subdirectories
add_subdirectory(src/odfaeg)
if(ODFAEG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
if(ODFAEG_BUILD_DOC)
    add_subdirectory(doc)
endif()

# setup the install rules
if(NOT ODFAEG_BUILD_FRAMEWORKS)
    install(DIRECTORY include
            DESTINATION .
            COMPONENT devel
            PATTERN ".svn" EXCLUDE)
else()
    # find only "root" headers
    file(GLOB ODFAEG_HEADERS RELATIVE ${PROJECT_SOURCE_DIR} "include/ODFAEG/*")

    # in fact we have to fool cmake to copy all the headers in subdirectories
    # to do that we have to add the "root" headers to the PUBLIC_HEADER
    # then we can run a post script to copy the remaining headers

    # we need a dummy file in order to compile the framework
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
                       COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

    set(ODFAEG_SOURCES ${ODFAEG_HEADERS})
    list(APPEND ODFAEG_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

    # create ODFAEG.framework
    add_library(ODFAEG ${ODFAEG_SOURCES})
	
    # edit target properties
    set_target_properties(ODFAEG PROPERTIES 
                          FRAMEWORK TRUE
                          FRAMEWORK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          MACOSX_FRAMEWORK_IDENTIFIER org.ODFAEG-dev.ODFAEG
                          MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          MACOSX_FRAMEWORK_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                          PUBLIC_HEADER "${ODFAEG_HEADERS}")

    # add the remaining headers
    add_custom_command(TARGET ODFAEG 
                       POST_BUILD
                       COMMAND cp -r ${PROJECT_SOURCE_DIR}/include/ODFAEG/* ODFAEG.framework/Versions/${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}/Headers)

    # adapt install directory to allow distributing dylibs/frameworks in user’s frameworks/application bundle
    # NOTE : it's not required to link agains ODFAEG.framework
    set_target_properties(ODFAEG PROPERTIES 
                          BUILD_WITH_INSTALL_RPATH 1 
                          INSTALL_NAME_DIR "@executable_path/../Frameworks")

    # install rule
    install(TARGETS ODFAEG
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_FRAMEWORK_PREFIX}
            COMPONENT devel)
endif()

install(FILES cmake/Modules/FindODFAEG.cmake DESTINATION ${INSTALL_MISC_DIR}/cmake/Modules)
install(FILES license.txt DESTINATION ${INSTALL_MISC_DIR})
install(FILES readme.txt DESTINATION ${INSTALL_MISC_DIR})

if(WINDOWS)	
	
    if(ARCH_32BITS)
		
		set(RCFLAGS "-F pe-i386")
		FILE(GLOB_RECURSE files ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/headers)
		install(FILES ${files} DESTINATION extlibs/headers)
		install(FILES extlibs/bin/x86/sfml-system-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/bin/x86/sfml-system-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/bin/x86/sfml-window-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/bin/x86/sfml-graphics-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/bin/x86/sfml-network-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/bin/x86/sfml-audio-2.dll DESTINATION extlibs/bin/x86)
		install(FILES extlibs/libs-mingw/x86/libsfml-system.a DESTINATION extlibs/libs-mingw/x86)
		install(FILES extlibs/libs-mingw/x86/libsfml-window.a DESTINATION extlibs/libs-mingw/x86)
		install(FILES extlibs/libs-mingw/x86/libsfml-graphics.a DESTINATION extlibs/libs-mingw/x86)
		install(FILES extlibs/libs-mingw/x86/libsfml-network.a DESTINATION extlibs/libs-mingw/x86b)
		install(FILES extlibs/libs-mingw/x86/libsfml-audio.a DESTINATION extlibs/libs-mingw/x86)
        install(FILES extlibs/bin/x86/libsndfile-1.dll DESTINATION extlibs/bin/x86)
        install(FILES extlibs/bin/x86/openal32.dll DESTINATION extlibs/bin/x86)		
    elseif(ARCH_64BITS)
        install(FILES extlibs/bin/x64/libsndfile-1.dll DESTINATION extlibs/bin/x86)
        install(FILES extlibs/bin/x64/openal32.dll DESTINATION extlibs/bin/x86)
    endif()
elseif(MACOSX)
    install(DIRECTORY extlibs/libs-osx/Frameworks/sndfile.framework DESTINATION ${CMAKE_INSTALL_FRAMEWORK_PREFIX})
    install(DIRECTORY extlibs/libs-osx/Frameworks/freetype.framework DESTINATION ${CMAKE_INSTALL_FRAMEWORK_PREFIX})
    
endif()

