# This script locates the SFGL library
# ------------------------------------
#
# Usage
# -----
#
# When you try to locate the SFGL libraries, you must specify which modules you want to use (system, window, graphics, network, audio, main).
# If none is given, the SFGL_LIBRARIES variable will be empty and you'll end up linking to nothing.
# example:
#   find_package(SFGL COMPONENTS graphics window system) // find the graphics, window and system modules
#
# You can enforce a specific version, either MAJOR.MINOR or only MAJOR.
# If nothing is specified, the version won't be checked (ie. any version will be accepted).
# example:
#   find_package(SFGL COMPONENTS ...)     // no specific version required
#   find_package(SFGL 2 COMPONENTS ...)   // any 2.x version
#   find_package(SFGL 2.4 COMPONENTS ...) // version 2.4 or greater
#
# By default, the dynamic libraries of SFGL will be found. To find the static ones instead,
# you must set the SFGL_STATIC_LIBRARIES variable to TRUE before calling find_package(SFGL ...).
# In case of static linking, the SFGL_STATIC macro will also be defined by this script.
# example:
#   set(SFGL_STATIC_LIBRARIES TRUE)
#   find_package(SFGL 2 COMPONENTS network system)
#
# On Mac OS X if SFGL_STATIC_LIBRARIES is not set to TRUE then by default CMake will search for frameworks unless
# CMAKE_FIND_FRAMEWORK is set to "NEVER" for example. Please refer to CMake documentation for more details.
# Moreover, keep in mind that SFGL frameworks are only available as release libraries unlike dylibs which
# are available for both release and debug modes.
#
# If SFGL is not installed in a standard path, you can use the SFGL_ROOT CMake (or environment) variable
# to tell CMake where SFGL is.
#
# Output
# ------
#
# This script defines the following variables:
# - For each specified module XXX (system, window, graphics, network, audio, main):
#   - SFGL_XXX_LIBRARY_DEBUG:   the name of the debug library of the xxx module (set to SFGL_XXX_LIBRARY_RELEASE is no debug version is found)
#   - SFGL_XXX_LIBRARY_RELEASE: the name of the release library of the xxx module (set to SFGL_XXX_LIBRARY_DEBUG is no release version is found)
#   - SFGL_XXX_LIBRARY:         the name of the library to link to for the xxx module (includes both debug and optimized names if necessary)
#   - SFGL_XXX_FOUND:           true if either the debug or release library of the xxx module is found
# - SFGL_LIBRARIES:   the list of all libraries corresponding to the required modules
# - SFGL_FOUND:       true if all the required modules are found
# - SFGL_INCLUDE_DIR: the path where SFGL headers are located (the directory containing the SFGL/Config.hpp file)
#
# example:
#   find_package(SFGL 2 COMPONENTS system window graphics audio REQUIRED)
#   include_directories(${SFGL_INCLUDE_DIR})
#   add_executable(myapp ...)
#   target_link_libraries(myapp ${SFGL_LIBRARIES})

# define the SFGL_STATIC macro if static build was chosen
if(SFGL_STATIC_LIBRARIES)
    add_definitions(-DSFGL_STATIC)
endif()

# deduce the libraries suffix from the options
set(FIND_SFGL_LIB_SUFFIX "")
if(SFGL_STATIC_LIBRARIES)
    set(FIND_SFGL_LIB_SUFFIX "${FIND_SFGL_LIB_SUFFIX}-s")
endif()

# find the SFGL include directory
find_path(SFGL_INCLUDE_DIR SFGL/Config.hpp
          PATH_SUFFIXES include
          PATHS
          ${SFGL_ROOT}
          $ENV{SFGL_ROOT}
          ~/Library/Frameworks
          /Library/Frameworks
          /usr/local/
          /usr/
          /sw          # Fink
          /opt/local/  # DarwinPorts
          /opt/csw/    # Blastwave
          /opt/)
		  

# check the version number
set(SFGL_VERSION_OK TRUE)
if(SFGL_FIND_VERSION AND SFGL_INCLUDE_DIR)
    # extract the major and minor version numbers from SFGL/Config.hpp
    # we have to handle framework a little bit differently :
    if("${SFGL_INCLUDE_DIR}" MATCHES "SFGL.framework")
        set(SFGL_CONFIG_HPP_INPUT "${SFGL_INCLUDE_DIR}/Headers/Config.hpp")
    else()
        set(SFGL_CONFIG_HPP_INPUT "${SFGL_INCLUDE_DIR}/SFGL/Config.hpp")
    endif()
    FILE(READ "${SFGL_CONFIG_HPP_INPUT}" SFGL_CONFIG_HPP_CONTENTS)
    STRING(REGEX MATCH ".*#define SFGL_VERSION_MAJOR ([0-9]+).*#define SFGL_VERSION_MINOR ([0-9]+).*" SFGL_CONFIG_HPP_CONTENTS "${SFGL_CONFIG_HPP_CONTENTS}")
    STRING(REGEX REPLACE ".*#define SFGL_VERSION_MAJOR ([0-9]+).*" "\\1" SFGL_VERSION_MAJOR "${SFGL_CONFIG_HPP_CONTENTS}")
    STRING(REGEX REPLACE ".*#define SFGL_VERSION_MINOR ([0-9]+).*" "\\1" SFGL_VERSION_MINOR "${SFGL_CONFIG_HPP_CONTENTS}")
    math(EXPR SFGL_REQUESTED_VERSION "${SFGL_FIND_VERSION_MAJOR} * 10 + ${SFGL_FIND_VERSION_MINOR}")

    # if we could extract them, compare with the requested version number
    if (SFGL_VERSION_MAJOR)
        # transform version numbers to an integer
        math(EXPR SFGL_VERSION "${SFGL_VERSION_MAJOR} * 10 + ${SFGL_VERSION_MINOR}")

        # compare them
        if(SFGL_VERSION LESS SFGL_REQUESTED_VERSION)
            set(SFGL_VERSION_OK FALSE)
        endif()
    else()
        # SFGL version is < 2.0
        if (SFGL_REQUESTED_VERSION GREATER 19)
            set(SFGL_VERSION_OK FALSE)
            set(SFGL_VERSION_MAJOR 1)
            set(SFGL_VERSION_MINOR x)
        endif()
    endif()
endif()

# find the requested modules
set(SFGL_FOUND TRUE) # will be set to false if one of the required modules is not found
set(FIND_SFGL_LIB_PATHS
    ${SFGL_ROOT}
    $ENV{SFGL_ROOT}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /sw
    /opt/local
    /opt/csw
    /opt)
	
foreach(FIND_SFGL_COMPONENT ${SFGL_FIND_COMPONENTS})
    string(TOLOWER ${FIND_SFGL_COMPONENT} FIND_SFGL_COMPONENT_LOWER)
    string(TOUPPER ${FIND_SFGL_COMPONENT} FIND_SFGL_COMPONENT_UPPER)
    set(FIND_SFGL_COMPONENT_NAME SFGL-${FIND_SFGL_COMPONENT_LOWER}${FIND_SFGL_LIB_SUFFIX})

    # no suffix for SFGL-main, it is always a static library
    if(FIND_SFGL_COMPONENT_LOWER STREQUAL "main")
        set(FIND_SFGL_COMPONENT_NAME SFGL-${FIND_SFGL_COMPONENT_LOWER})
    endif()

    # debug library
    find_library(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG
                 NAMES ${FIND_SFGL_COMPONENT_NAME}-d
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_SFGL_LIB_PATHS})

    # release library
    find_library(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE
                 NAMES ${FIND_SFGL_COMPONENT_NAME}
                 PATH_SUFFIXES lib64 lib
                 PATHS ${FIND_SFGL_LIB_PATHS})

    if (SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG OR SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE)
        # library found
        set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_FOUND TRUE)
        
        # if both are found, set SFGL_XXX_LIBRARY to contain both
        if (SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG AND SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE)
            set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY debug     ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG}
                                                          optimized ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE})
        endif()

        # if only one debug/release variant is found, set the other to be equal to the found one
        if (SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG AND NOT SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE)
            # debug and not release
            set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG})
            set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY         ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG})
        endif()
        if (SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE AND NOT SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG)
            # release and not debug
            set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE})
            set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY       ${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE})
        endif()
    else()
        # library not found
        set(SFGL_FOUND FALSE)
        set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_FOUND FALSE)
        set(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY "")
        set(FIND_SFGL_MISSING "${FIND_SFGL_MISSING} SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY")
    endif()

    # mark as advanced
    MARK_AS_ADVANCED(SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY
                     SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_RELEASE
                     SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY_DEBUG)

    # add to the global list of libraries
    set(SFGL_LIBRARIES ${SFGL_LIBRARIES} "${SFGL_${FIND_SFGL_COMPONENT_UPPER}_LIBRARY}")
endforeach()

# handle errors
if(NOT SFGL_VERSION_OK)
    # SFGL version not ok
    set(FIND_SFGL_ERROR "SFGL found but version too low (requested: ${SFGL_FIND_VERSION}, found: ${SFGL_VERSION_MAJOR}.${SFGL_VERSION_MINOR})")
    set(SFGL_FOUND FALSE)
elseif(NOT SFGL_FOUND)
    # include directory or library not found
    set(FIND_SFGL_ERROR "Could NOT find SFGL (missing: ${FIND_SFGL_MISSING})")
endif()
if (NOT SFGL_FOUND)
    if(SFGL_FIND_REQUIRED)
        # fatal error
        message(FATAL_ERROR ${FIND_SFGL_ERROR})
    elseif(NOT SFGL_FIND_QUIETLY)
        # error but continue
        message("${FIND_SFGL_ERROR}")
    endif()
endif()

# handle success
if(SFGL_FOUND)
    message(STATUS "Found SFGL ${SFGL_VERSION_MAJOR}.${SFGL_VERSION_MINOR} in ${SFGL_INCLUDE_DIR}")
endif()
