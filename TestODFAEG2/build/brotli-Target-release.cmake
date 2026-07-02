# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(brotli_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(brotli_FRAMEWORKS_FOUND_RELEASE "${brotli_FRAMEWORKS_RELEASE}" "${brotli_FRAMEWORK_DIRS_RELEASE}")

set(brotli_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET brotli_DEPS_TARGET)
    add_library(brotli_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET brotli_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${brotli_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${brotli_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:brotli::brotlicommon>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### brotli_DEPS_TARGET to all of them
conan_package_library_targets("${brotli_LIBS_RELEASE}"    # libraries
                              "${brotli_LIB_DIRS_RELEASE}" # package_libdir
                              "${brotli_BIN_DIRS_RELEASE}" # package_bindir
                              "${brotli_LIBRARY_TYPE_RELEASE}"
                              "${brotli_IS_HOST_WINDOWS_RELEASE}"
                              brotli_DEPS_TARGET
                              brotli_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "brotli"    # package_name
                              "${brotli_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${brotli_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Release ########################################

    ########## COMPONENT brotli::brotlienc #############

        set(brotli_brotli_brotlienc_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(brotli_brotli_brotlienc_FRAMEWORKS_FOUND_RELEASE "${brotli_brotli_brotlienc_FRAMEWORKS_RELEASE}" "${brotli_brotli_brotlienc_FRAMEWORK_DIRS_RELEASE}")

        set(brotli_brotli_brotlienc_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET brotli_brotli_brotlienc_DEPS_TARGET)
            add_library(brotli_brotli_brotlienc_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET brotli_brotli_brotlienc_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'brotli_brotli_brotlienc_DEPS_TARGET' to all of them
        conan_package_library_targets("${brotli_brotli_brotlienc_LIBS_RELEASE}"
                              "${brotli_brotli_brotlienc_LIB_DIRS_RELEASE}"
                              "${brotli_brotli_brotlienc_BIN_DIRS_RELEASE}" # package_bindir
                              "${brotli_brotli_brotlienc_LIBRARY_TYPE_RELEASE}"
                              "${brotli_brotli_brotlienc_IS_HOST_WINDOWS_RELEASE}"
                              brotli_brotli_brotlienc_DEPS_TARGET
                              brotli_brotli_brotlienc_LIBRARIES_TARGETS
                              "_RELEASE"
                              "brotli_brotli_brotlienc"
                              "${brotli_brotli_brotlienc_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET brotli::brotlienc
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_LIBRARIES_TARGETS}>
                     )

        if("${brotli_brotli_brotlienc_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET brotli::brotlienc
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         brotli_brotli_brotlienc_DEPS_TARGET)
        endif()

        set_property(TARGET brotli::brotlienc APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET brotli::brotlienc APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlienc APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_LIB_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlienc APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET brotli::brotlienc APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlienc_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT brotli::brotlidec #############

        set(brotli_brotli_brotlidec_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(brotli_brotli_brotlidec_FRAMEWORKS_FOUND_RELEASE "${brotli_brotli_brotlidec_FRAMEWORKS_RELEASE}" "${brotli_brotli_brotlidec_FRAMEWORK_DIRS_RELEASE}")

        set(brotli_brotli_brotlidec_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET brotli_brotli_brotlidec_DEPS_TARGET)
            add_library(brotli_brotli_brotlidec_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET brotli_brotli_brotlidec_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'brotli_brotli_brotlidec_DEPS_TARGET' to all of them
        conan_package_library_targets("${brotli_brotli_brotlidec_LIBS_RELEASE}"
                              "${brotli_brotli_brotlidec_LIB_DIRS_RELEASE}"
                              "${brotli_brotli_brotlidec_BIN_DIRS_RELEASE}" # package_bindir
                              "${brotli_brotli_brotlidec_LIBRARY_TYPE_RELEASE}"
                              "${brotli_brotli_brotlidec_IS_HOST_WINDOWS_RELEASE}"
                              brotli_brotli_brotlidec_DEPS_TARGET
                              brotli_brotli_brotlidec_LIBRARIES_TARGETS
                              "_RELEASE"
                              "brotli_brotli_brotlidec"
                              "${brotli_brotli_brotlidec_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET brotli::brotlidec
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_LIBRARIES_TARGETS}>
                     )

        if("${brotli_brotli_brotlidec_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET brotli::brotlidec
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         brotli_brotli_brotlidec_DEPS_TARGET)
        endif()

        set_property(TARGET brotli::brotlidec APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET brotli::brotlidec APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlidec APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_LIB_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlidec APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET brotli::brotlidec APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlidec_COMPILE_OPTIONS_RELEASE}>)


    ########## COMPONENT brotli::brotlicommon #############

        set(brotli_brotli_brotlicommon_FRAMEWORKS_FOUND_RELEASE "")
        conan_find_apple_frameworks(brotli_brotli_brotlicommon_FRAMEWORKS_FOUND_RELEASE "${brotli_brotli_brotlicommon_FRAMEWORKS_RELEASE}" "${brotli_brotli_brotlicommon_FRAMEWORK_DIRS_RELEASE}")

        set(brotli_brotli_brotlicommon_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET brotli_brotli_brotlicommon_DEPS_TARGET)
            add_library(brotli_brotli_brotlicommon_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET brotli_brotli_brotlicommon_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_FRAMEWORKS_FOUND_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_SYSTEM_LIBS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_DEPENDENCIES_RELEASE}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'brotli_brotli_brotlicommon_DEPS_TARGET' to all of them
        conan_package_library_targets("${brotli_brotli_brotlicommon_LIBS_RELEASE}"
                              "${brotli_brotli_brotlicommon_LIB_DIRS_RELEASE}"
                              "${brotli_brotli_brotlicommon_BIN_DIRS_RELEASE}" # package_bindir
                              "${brotli_brotli_brotlicommon_LIBRARY_TYPE_RELEASE}"
                              "${brotli_brotli_brotlicommon_IS_HOST_WINDOWS_RELEASE}"
                              brotli_brotli_brotlicommon_DEPS_TARGET
                              brotli_brotli_brotlicommon_LIBRARIES_TARGETS
                              "_RELEASE"
                              "brotli_brotli_brotlicommon"
                              "${brotli_brotli_brotlicommon_NO_SONAME_MODE_RELEASE}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET brotli::brotlicommon
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_OBJECTS_RELEASE}>
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_LIBRARIES_TARGETS}>
                     )

        if("${brotli_brotli_brotlicommon_LIBS_RELEASE}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET brotli::brotlicommon
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         brotli_brotli_brotlicommon_DEPS_TARGET)
        endif()

        set_property(TARGET brotli::brotlicommon APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_LINKER_FLAGS_RELEASE}>)
        set_property(TARGET brotli::brotlicommon APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_INCLUDE_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlicommon APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_LIB_DIRS_RELEASE}>)
        set_property(TARGET brotli::brotlicommon APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_COMPILE_DEFINITIONS_RELEASE}>)
        set_property(TARGET brotli::brotlicommon APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Release>:${brotli_brotli_brotlicommon_COMPILE_OPTIONS_RELEASE}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET brotli::brotli APPEND PROPERTY INTERFACE_LINK_LIBRARIES brotli::brotlienc)
    set_property(TARGET brotli::brotli APPEND PROPERTY INTERFACE_LINK_LIBRARIES brotli::brotlidec)
    set_property(TARGET brotli::brotli APPEND PROPERTY INTERFACE_LINK_LIBRARIES brotli::brotlicommon)

########## For the modules (FindXXX)
set(brotli_LIBRARIES_RELEASE brotli::brotli)
