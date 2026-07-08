# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(imgui_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(imgui_FRAMEWORKS_FOUND_RELEASE "${imgui_FRAMEWORKS_RELEASE}" "${imgui_FRAMEWORK_DIRS_RELEASE}")

set(imgui_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET imgui_DEPS_TARGET)
    add_library(imgui_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET imgui_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${imgui_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${imgui_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### imgui_DEPS_TARGET to all of them
conan_package_library_targets("${imgui_LIBS_RELEASE}"    # libraries
                              "${imgui_LIB_DIRS_RELEASE}" # package_libdir
                              "${imgui_BIN_DIRS_RELEASE}" # package_bindir
                              "${imgui_LIBRARY_TYPE_RELEASE}"
                              "${imgui_IS_HOST_WINDOWS_RELEASE}"
                              imgui_DEPS_TARGET
                              imgui_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "imgui"    # package_name
                              "${imgui_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${imgui_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${imgui_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${imgui_LIBRARIES_TARGETS}>
                 )

    if("${imgui_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET imgui::imgui
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     imgui_DEPS_TARGET)
    endif()

    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${imgui_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${imgui_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${imgui_LIB_DIRS_RELEASE}>)
    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${imgui_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET imgui::imgui
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${imgui_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(imgui_LIBRARIES_RELEASE imgui::imgui)
