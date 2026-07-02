########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(assimp_FIND_QUIETLY)
    set(assimp_MESSAGE_MODE VERBOSE)
else()
    set(assimp_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/assimpTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${assimp_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(assimp_VERSION_STRING "5.3.1")
set(assimp_INCLUDE_DIRS ${assimp_INCLUDE_DIRS_RELEASE} )
set(assimp_INCLUDE_DIR ${assimp_INCLUDE_DIRS_RELEASE} )
set(assimp_LIBRARIES ${assimp_LIBRARIES_RELEASE} )
set(assimp_DEFINITIONS ${assimp_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${assimp_BUILD_MODULES_PATHS_RELEASE} )
    message(${assimp_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


