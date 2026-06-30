# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(opus_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(opus_FRAMEWORKS_FOUND_RELEASE "${opus_FRAMEWORKS_RELEASE}" "${opus_FRAMEWORK_DIRS_RELEASE}")

set(opus_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET opus_DEPS_TARGET)
    add_library(opus_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET opus_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${opus_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${opus_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### opus_DEPS_TARGET to all of them
conan_package_library_targets("${opus_LIBS_RELEASE}"    # libraries
                              "${opus_LIB_DIRS_RELEASE}" # package_libdir
                              "${opus_BIN_DIRS_RELEASE}" # package_bindir
                              "${opus_LIBRARY_TYPE_RELEASE}"
                              "${opus_IS_HOST_WINDOWS_RELEASE}"
                              opus_DEPS_TARGET
                              opus_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "opus"    # package_name
                              "${opus_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${opus_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT Opus::opus #############

        set(opus_Opus_opus_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(opus_Opus_opus_FRAMEWORKS_FOUND_RELEASE "${opus_Opus_opus_FRAMEWORKS_RELEASE}" "${opus_Opus_opus_FRAMEWORK_DIRS_RELEASE}")

        set(opus_Opus_opus_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET opus_Opus_opus_DEPS_TARGET)
            add_library(opus_Opus_opus_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET opus_Opus_opus_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${opus_Opus_opus_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${opus_Opus_opus_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${opus_Opus_opus_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'opus_Opus_opus_DEPS_TARGET' to all of them
        conan_package_library_targets("${opus_Opus_opus_LIBS_RELEASE}"
                              "${opus_Opus_opus_LIB_DIRS_RELEASE}"
                              "${opus_Opus_opus_BIN_DIRS_RELEASE}" # package_bindir
                              "${opus_Opus_opus_LIBRARY_TYPE_RELEASE}"
                              "${opus_Opus_opus_IS_HOST_WINDOWS_RELEASE}"
                              opus_Opus_opus_DEPS_TARGET
                              opus_Opus_opus_LIBRARIES_TARGETS
                              "_RELEASE"
                              "opus_Opus_opus"
                              "${opus_Opus_opus_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET Opus::opus
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${opus_Opus_opus_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${opus_Opus_opus_LIBRARIES_TARGETS}>
                     )

        if("${opus_Opus_opus_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET Opus::opus
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         opus_Opus_opus_DEPS_TARGET)
        endif()

        set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${opus_Opus_opus_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${opus_Opus_opus_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${opus_Opus_opus_LIB_DIRS_RELEASE}>)
        set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${opus_Opus_opus_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${opus_Opus_opus_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET Opus::opus APPEND PROPERTY INTERFACE_LINK_LIBRARIES Opus::opus)

########## For the modules (FindXXX)
set(opus_LIBRARIES_RELEASE Opus::opus)
