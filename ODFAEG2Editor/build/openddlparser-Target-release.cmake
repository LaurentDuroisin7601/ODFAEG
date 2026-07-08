# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(openddl-parser_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(openddl-parser_FRAMEWORKS_FOUND_RELEASE "${openddl-parser_FRAMEWORKS_RELEASE}" "${openddl-parser_FRAMEWORK_DIRS_RELEASE}")

set(openddl-parser_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET openddl-parser_DEPS_TARGET)
    add_library(openddl-parser_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET openddl-parser_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${openddl-parser_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${openddl-parser_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### openddl-parser_DEPS_TARGET to all of them
conan_package_library_targets("${openddl-parser_LIBS_RELEASE}"    # libraries
                              "${openddl-parser_LIB_DIRS_RELEASE}" # package_libdir
                              "${openddl-parser_BIN_DIRS_RELEASE}" # package_bindir
                              "${openddl-parser_LIBRARY_TYPE_RELEASE}"
                              "${openddl-parser_IS_HOST_WINDOWS_RELEASE}"
                              openddl-parser_DEPS_TARGET
                              openddl-parser_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "openddl-parser"    # package_name
                              "${openddl-parser_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${openddl-parser_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${openddl-parser_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${openddl-parser_LIBRARIES_TARGETS}>
                 )

    if("${openddl-parser_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET openddlparser::openddlparser
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     openddl-parser_DEPS_TARGET)
    endif()

    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${openddl-parser_LINKER_FLAGS_RELEASE}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${openddl-parser_INCLUDE_DIRS_RELEASE}>)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${openddl-parser_LIB_DIRS_RELEASE}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${openddl-parser_COMPILE_DEFINITIONS_RELEASE}>)
    set_property(TARGET openddlparser::openddlparser
                 APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${openddl-parser_COMPILE_OPTIONS_RELEASE}>)

########## For the modules (FindXXX)
set(openddl-parser_LIBRARIES_RELEASE openddlparser::openddlparser)
