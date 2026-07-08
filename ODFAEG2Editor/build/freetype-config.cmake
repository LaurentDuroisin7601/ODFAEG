########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(freetype_FIND_QUIETLY)
    set(freetype_MESSAGE_MODE VERBOSE)
else()
    set(freetype_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/freetypeTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${freetype_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(FREETYPE_VERSION_STRING "2.13.2")
set(FREETYPE_INCLUDE_DIRS ${freetype_INCLUDE_DIRS_RELEASE} )
set(FREETYPE_INCLUDE_DIR ${freetype_INCLUDE_DIRS_RELEASE} )
set(FREETYPE_LIBRARIES ${freetype_LIBRARIES_RELEASE} )
set(FREETYPE_DEFINITIONS ${freetype_DEFINITIONS_RELEASE} )

set(freetype_VERSION_STRING "2.13.2")
set(freetype_INCLUDE_DIRS ${freetype_INCLUDE_DIRS_RELEASE} )
set(freetype_INCLUDE_DIR ${freetype_INCLUDE_DIRS_RELEASE} )
set(freetype_LIBRARIES ${freetype_LIBRARIES_RELEASE} )
set(freetype_DEFINITIONS ${freetype_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${freetype_BUILD_MODULES_PATHS_RELEASE} )
    message(${freetype_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


