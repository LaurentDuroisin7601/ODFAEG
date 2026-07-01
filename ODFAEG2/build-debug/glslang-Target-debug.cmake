# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(glslang_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(glslang_FRAMEWORKS_FOUND_DEBUG "${glslang_FRAMEWORKS_DEBUG}" "${glslang_FRAMEWORK_DIRS_DEBUG}")

set(glslang_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET glslang_DEPS_TARGET)
    add_library(glslang_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET glslang_DEPS_TARGET
             APPEND PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${glslang_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${glslang_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:glslang::MachineIndependent;glslang::GenericCodeGen;glslang::OSDependent;glslang::glslang;SPIRV-Tools-opt>)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### glslang_DEPS_TARGET to all of them
conan_package_library_targets("${glslang_LIBS_DEBUG}"    # libraries
                              "${glslang_LIB_DIRS_DEBUG}" # package_libdir
                              "${glslang_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_LIBRARY_TYPE_DEBUG}"
                              "${glslang_IS_HOST_WINDOWS_DEBUG}"
                              glslang_DEPS_TARGET
                              glslang_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "glslang"    # package_name
                              "${glslang_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${glslang_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT glslang::SPIRV #############

        set(glslang_glslang_SPIRV_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_SPIRV_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_SPIRV_FRAMEWORKS_DEBUG}" "${glslang_glslang_SPIRV_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_SPIRV_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_SPIRV_DEPS_TARGET)
            add_library(glslang_glslang_SPIRV_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_SPIRV_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_SPIRV_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_SPIRV_LIBS_DEBUG}"
                              "${glslang_glslang_SPIRV_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_SPIRV_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_SPIRV_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_SPIRV_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_SPIRV_DEPS_TARGET
                              glslang_glslang_SPIRV_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_SPIRV"
                              "${glslang_glslang_SPIRV_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::SPIRV
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_SPIRV_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::SPIRV
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_SPIRV_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::SPIRV APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::SPIRV APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::SPIRV APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::SPIRV APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::SPIRV APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPIRV_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::glslang #############

        set(glslang_glslang_glslang_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_glslang_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_glslang_FRAMEWORKS_DEBUG}" "${glslang_glslang_glslang_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_glslang_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_glslang_DEPS_TARGET)
            add_library(glslang_glslang_glslang_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_glslang_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_glslang_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_glslang_LIBS_DEBUG}"
                              "${glslang_glslang_glslang_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_glslang_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_glslang_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_glslang_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_glslang_DEPS_TARGET
                              glslang_glslang_glslang_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_glslang"
                              "${glslang_glslang_glslang_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::glslang
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_glslang_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::glslang
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_glslang_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::glslang APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::glslang APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::glslang APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::glslang APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::glslang APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::MachineIndependent #############

        set(glslang_glslang_MachineIndependent_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_MachineIndependent_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_MachineIndependent_FRAMEWORKS_DEBUG}" "${glslang_glslang_MachineIndependent_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_MachineIndependent_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_MachineIndependent_DEPS_TARGET)
            add_library(glslang_glslang_MachineIndependent_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_MachineIndependent_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_MachineIndependent_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_MachineIndependent_LIBS_DEBUG}"
                              "${glslang_glslang_MachineIndependent_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_MachineIndependent_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_MachineIndependent_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_MachineIndependent_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_MachineIndependent_DEPS_TARGET
                              glslang_glslang_MachineIndependent_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_MachineIndependent"
                              "${glslang_glslang_MachineIndependent_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::MachineIndependent
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_MachineIndependent_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::MachineIndependent
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_MachineIndependent_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::MachineIndependent APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::MachineIndependent APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::MachineIndependent APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::MachineIndependent APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::MachineIndependent APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_MachineIndependent_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::glslang-default-resource-limits #############

        set(glslang_glslang_glslang-default-resource-limits_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_glslang-default-resource-limits_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_glslang-default-resource-limits_FRAMEWORKS_DEBUG}" "${glslang_glslang_glslang-default-resource-limits_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_glslang-default-resource-limits_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_glslang-default-resource-limits_DEPS_TARGET)
            add_library(glslang_glslang_glslang-default-resource-limits_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_glslang-default-resource-limits_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_glslang-default-resource-limits_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_glslang-default-resource-limits_LIBS_DEBUG}"
                              "${glslang_glslang_glslang-default-resource-limits_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_glslang-default-resource-limits_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_glslang-default-resource-limits_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_glslang-default-resource-limits_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_glslang-default-resource-limits_DEPS_TARGET
                              glslang_glslang_glslang-default-resource-limits_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_glslang-default-resource-limits"
                              "${glslang_glslang_glslang-default-resource-limits_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::glslang-default-resource-limits
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_glslang-default-resource-limits_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::glslang-default-resource-limits
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_glslang-default-resource-limits_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::glslang-default-resource-limits APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::SPVRemapper #############

        set(glslang_glslang_SPVRemapper_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_SPVRemapper_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_SPVRemapper_FRAMEWORKS_DEBUG}" "${glslang_glslang_SPVRemapper_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_SPVRemapper_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_SPVRemapper_DEPS_TARGET)
            add_library(glslang_glslang_SPVRemapper_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_SPVRemapper_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_SPVRemapper_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_SPVRemapper_LIBS_DEBUG}"
                              "${glslang_glslang_SPVRemapper_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_SPVRemapper_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_SPVRemapper_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_SPVRemapper_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_SPVRemapper_DEPS_TARGET
                              glslang_glslang_SPVRemapper_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_SPVRemapper"
                              "${glslang_glslang_SPVRemapper_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::SPVRemapper
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_SPVRemapper_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::SPVRemapper
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_SPVRemapper_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::SPVRemapper APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::SPVRemapper APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::SPVRemapper APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::SPVRemapper APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::SPVRemapper APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_SPVRemapper_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::OSDependent #############

        set(glslang_glslang_OSDependent_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_OSDependent_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_OSDependent_FRAMEWORKS_DEBUG}" "${glslang_glslang_OSDependent_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_OSDependent_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_OSDependent_DEPS_TARGET)
            add_library(glslang_glslang_OSDependent_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_OSDependent_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_OSDependent_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_OSDependent_LIBS_DEBUG}"
                              "${glslang_glslang_OSDependent_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_OSDependent_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_OSDependent_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_OSDependent_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_OSDependent_DEPS_TARGET
                              glslang_glslang_OSDependent_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_OSDependent"
                              "${glslang_glslang_OSDependent_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::OSDependent
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_OSDependent_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::OSDependent
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_OSDependent_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::OSDependent APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::OSDependent APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::OSDependent APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::OSDependent APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::OSDependent APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_OSDependent_COMPILE_OPTIONS_DEBUG}>)


    ########## COMPONENT glslang::GenericCodeGen #############

        set(glslang_glslang_GenericCodeGen_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(glslang_glslang_GenericCodeGen_FRAMEWORKS_FOUND_DEBUG "${glslang_glslang_GenericCodeGen_FRAMEWORKS_DEBUG}" "${glslang_glslang_GenericCodeGen_FRAMEWORK_DIRS_DEBUG}")

        set(glslang_glslang_GenericCodeGen_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET glslang_glslang_GenericCodeGen_DEPS_TARGET)
            add_library(glslang_glslang_GenericCodeGen_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET glslang_glslang_GenericCodeGen_DEPS_TARGET
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_DEPENDENCIES_DEBUG}>
                     )

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'glslang_glslang_GenericCodeGen_DEPS_TARGET' to all of them
        conan_package_library_targets("${glslang_glslang_GenericCodeGen_LIBS_DEBUG}"
                              "${glslang_glslang_GenericCodeGen_LIB_DIRS_DEBUG}"
                              "${glslang_glslang_GenericCodeGen_BIN_DIRS_DEBUG}" # package_bindir
                              "${glslang_glslang_GenericCodeGen_LIBRARY_TYPE_DEBUG}"
                              "${glslang_glslang_GenericCodeGen_IS_HOST_WINDOWS_DEBUG}"
                              glslang_glslang_GenericCodeGen_DEPS_TARGET
                              glslang_glslang_GenericCodeGen_LIBRARIES_TARGETS
                              "_DEBUG"
                              "glslang_glslang_GenericCodeGen"
                              "${glslang_glslang_GenericCodeGen_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET glslang::GenericCodeGen
                     APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_LIBRARIES_TARGETS}>
                     )

        if("${glslang_glslang_GenericCodeGen_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET glslang::GenericCodeGen
                         APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                         glslang_glslang_GenericCodeGen_DEPS_TARGET)
        endif()

        set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_LINKER_FLAGS_DEBUG}>)
        set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_INCLUDE_DIRS_DEBUG}>)
        set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_LIB_DIRS_DEBUG}>)
        set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_COMPILE_DEFINITIONS_DEBUG}>)
        set_property(TARGET glslang::GenericCodeGen APPEND PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_DEBUG}>)


    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::SPIRV)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::glslang)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::MachineIndependent)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::glslang-default-resource-limits)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::SPVRemapper)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::OSDependent)
    set_property(TARGET glslang::_glslang-do-not-use APPEND PROPERTY INTERFACE_LINK_LIBRARIES glslang::GenericCodeGen)

########## For the modules (FindXXX)
set(glslang_LIBRARIES_DEBUG glslang::_glslang-do-not-use)
