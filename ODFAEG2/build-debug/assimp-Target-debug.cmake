# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(assimp_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(assimp_FRAMEWORKS_FOUND_DEBUG "${assimp_FRAMEWORKS_DEBUG}" "${assimp_FRAMEWORK_DIRS_DEBUG}")

set(assimp_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET assimp_DEPS_TARGET)
    add_library(assimp_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET assimp_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${assimp_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${assimp_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:minizip::minizip;pugixml::pugixml;zip::zip;poly2tri::poly2tri;draco::draco;clipper::clipper;openddlparser::openddlparser;ZLIB::ZLIB;stb::stb>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### assimp_DEPS_TARGET to all of them
conan_package_library_targets("${assimp_LIBS_DEBUG}"    # libraries
                              "${assimp_LIB_DIRS_DEBUG}" # package_libdir
                              "${assimp_BIN_DIRS_DEBUG}" # package_bindir
                              "${assimp_LIBRARY_TYPE_DEBUG}"
                              "${assimp_IS_HOST_WINDOWS_DEBUG}"
                              assimp_DEPS_TARGET
                              assimp_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "assimp"    # package_name
                              "${assimp_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${assimp_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${assimp_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${assimp_LIBRARIES_TARGETS}>
                 )

    if("${assimp_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET assimp::assimp
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     assimp_DEPS_TARGET)
    endif()

    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${assimp_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${assimp_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${assimp_LIB_DIRS_DEBUG}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${assimp_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET assimp::assimp
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${assimp_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(assimp_LIBRARIES_DEBUG assimp::assimp)
