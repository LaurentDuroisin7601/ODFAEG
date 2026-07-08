# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(shaderc_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(shaderc_FRAMEWORKS_FOUND_RELEASE "${shaderc_FRAMEWORKS_RELEASE}" "${shaderc_FRAMEWORK_DIRS_RELEASE}")

set(shaderc_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET shaderc_DEPS_TARGET)
    add_library(shaderc_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET shaderc_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${shaderc_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${shaderc_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:glslang::glslang;glslang::OSDependent;glslang::SPIRV;SPIRV-Tools-static;SPIRV-Tools-opt>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### shaderc_DEPS_TARGET to all of them
conan_package_library_targets("${shaderc_LIBS_RELEASE}"    # libraries
                              "${shaderc_LIB_DIRS_RELEASE}" # package_libdir
                              "${shaderc_BIN_DIRS_RELEASE}" # package_bindir
                              "${shaderc_LIBRARY_TYPE_RELEASE}"
                              "${shaderc_IS_HOST_WINDOWS_RELEASE}"
                              shaderc_DEPS_TARGET
                              shaderc_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "shaderc"    # package_name
                              "${shaderc_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${shaderc_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${shaderc_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${shaderc_LIBRARIES_TARGETS}>
                 )

    if("${shaderc_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET shaderc::shaderc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     shaderc_DEPS_TARGET)
    endif()

    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${shaderc_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${shaderc_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${shaderc_LIB_DIRS_RELEASE}>)
    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${shaderc_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET shaderc::shaderc
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${shaderc_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(shaderc_LIBRARIES_RELEASE shaderc::shaderc)
