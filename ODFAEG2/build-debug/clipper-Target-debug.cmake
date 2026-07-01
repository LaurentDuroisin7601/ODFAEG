# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(clipper_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(clipper_FRAMEWORKS_FOUND_DEBUG "${clipper_FRAMEWORKS_DEBUG}" "${clipper_FRAMEWORK_DIRS_DEBUG}")

set(clipper_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET clipper_DEPS_TARGET)
    add_library(clipper_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET clipper_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${clipper_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${clipper_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### clipper_DEPS_TARGET to all of them
conan_package_library_targets("${clipper_LIBS_DEBUG}"    # libraries
                              "${clipper_LIB_DIRS_DEBUG}" # package_libdir
                              "${clipper_BIN_DIRS_DEBUG}" # package_bindir
                              "${clipper_LIBRARY_TYPE_DEBUG}"
                              "${clipper_IS_HOST_WINDOWS_DEBUG}"
                              clipper_DEPS_TARGET
                              clipper_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "clipper"    # package_name
                              "${clipper_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${clipper_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${clipper_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${clipper_LIBRARIES_TARGETS}>
                 )

    if("${clipper_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET clipper::clipper
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     clipper_DEPS_TARGET)
    endif()

    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${clipper_LINKER_FLAGS_DEBUG}>)
    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${clipper_INCLUDE_DIRS_DEBUG}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${clipper_LIB_DIRS_DEBUG}>)
    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${clipper_COMPILE_DEFINITIONS_DEBUG}>)
    set_property(TARGET clipper::clipper
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${clipper_COMPILE_OPTIONS_DEBUG}>)

########## For the modules (FindXXX)
set(clipper_LIBRARIES_DEBUG clipper::clipper)
