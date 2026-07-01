# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(draco_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(draco_FRAMEWORKS_FOUND_DEBUG "${draco_FRAMEWORKS_DEBUG}" "${draco_FRAMEWORK_DIRS_DEBUG}")

set(draco_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET draco_DEPS_TARGET)
    add_library(draco_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET draco_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${draco_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${draco_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### draco_DEPS_TARGET to all of them
conan_package_library_targets("${draco_LIBS_DEBUG}"    # libraries
                              "${draco_LIB_DIRS_DEBUG}" # package_libdir
                              "${draco_BIN_DIRS_DEBUG}" # package_bindir
                              "${draco_LIBRARY_TYPE_DEBUG}"
                              "${draco_IS_HOST_WINDOWS_DEBUG}"
                              draco_DEPS_TARGET
                              draco_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "draco"    # package_name
                              "${draco_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${draco_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${draco_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${draco_LIBRARIES_TARGETS}>
                 )

    if("${draco_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET draco::draco
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     draco_DEPS_TARGET)
    endif()

    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${draco_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${draco_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${draco_LIB_DIRS_DEBUG}>)
    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${draco_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET draco::draco
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${draco_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(draco_LIBRARIES_DEBUG draco::draco)
