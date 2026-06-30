########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(openddlparser_FIND_QUIETLY)
    set(openddlparser_MESSAGE_MODE VERBOSE)
else()
    set(openddlparser_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/openddlparserTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${openddl-parser_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(openddlparser_VERSION_STRING "0.5.1")
set(openddlparser_INCLUDE_DIRS ${openddl-parser_INCLUDE_DIRS_RELEASE} )
set(openddlparser_INCLUDE_DIR ${openddl-parser_INCLUDE_DIRS_RELEASE} )
set(openddlparser_LIBRARIES ${openddl-parser_LIBRARIES_RELEASE} )
set(openddlparser_DEFINITIONS ${openddl-parser_DEFINITIONS_RELEASE} )


# Definition of extra CMake variables from cmake_extra_variables


# Only the last installed configuration BUILD_MODULES are included to avoid the collision
foreach(_BUILD_MODULE ${openddl-parser_BUILD_MODULES_PATHS_RELEASE} )
    message(${openddlparser_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


