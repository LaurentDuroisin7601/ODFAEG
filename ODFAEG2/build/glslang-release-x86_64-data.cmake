########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND glslang_COMPONENT_NAMES glslang::GenericCodeGen glslang::OSDependent glslang::SPVRemapper glslang::glslang-default-resource-limits glslang::MachineIndependent glslang::glslang glslang::SPIRV)
list(REMOVE_DUPLICATES glslang_COMPONENT_NAMES)
if(DEFINED glslang_FIND_DEPENDENCY_NAMES)
  list(APPEND glslang_FIND_DEPENDENCY_NAMES SPIRV-Tools)
  list(REMOVE_DUPLICATES glslang_FIND_DEPENDENCY_NAMES)
else()
  set(glslang_FIND_DEPENDENCY_NAMES SPIRV-Tools)
endif()
set(SPIRV-Tools_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(glslang_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/glslacdd7987ba73f6/p")
set(glslang_BUILD_MODULES_PATHS_RELEASE )


set(glslang_INCLUDE_DIRS_RELEASE )
set(glslang_RES_DIRS_RELEASE )
set(glslang_DEFINITIONS_RELEASE "-DENABLE_OPT")
set(glslang_SHARED_LINK_FLAGS_RELEASE )
set(glslang_EXE_LINK_FLAGS_RELEASE )
set(glslang_OBJECTS_RELEASE )
set(glslang_COMPILE_DEFINITIONS_RELEASE "ENABLE_OPT")
set(glslang_COMPILE_OPTIONS_C_RELEASE )
set(glslang_COMPILE_OPTIONS_CXX_RELEASE )
set(glslang_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_BIN_DIRS_RELEASE )
set(glslang_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_LIBS_RELEASE SPIRV glslang MachineIndependent glslang-default-resource-limits SPVRemapper OSDependent GenericCodeGen)
set(glslang_SYSTEM_LIBS_RELEASE m pthread)
set(glslang_FRAMEWORK_DIRS_RELEASE )
set(glslang_FRAMEWORKS_RELEASE )
set(glslang_BUILD_DIRS_RELEASE )
set(glslang_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(glslang_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_COMPILE_OPTIONS_C_RELEASE}>")
set(glslang_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_EXE_LINK_FLAGS_RELEASE}>")


set(glslang_COMPONENTS_RELEASE glslang::GenericCodeGen glslang::OSDependent glslang::SPVRemapper glslang::glslang-default-resource-limits glslang::MachineIndependent glslang::glslang glslang::SPIRV)
########### COMPONENT glslang::SPIRV VARIABLES ############################################

set(glslang_glslang_SPIRV_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_SPIRV_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_SPIRV_BIN_DIRS_RELEASE )
set(glslang_glslang_SPIRV_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_SPIRV_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_SPIRV_RES_DIRS_RELEASE )
set(glslang_glslang_SPIRV_DEFINITIONS_RELEASE "-DENABLE_OPT")
set(glslang_glslang_SPIRV_OBJECTS_RELEASE )
set(glslang_glslang_SPIRV_COMPILE_DEFINITIONS_RELEASE "ENABLE_OPT")
set(glslang_glslang_SPIRV_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_SPIRV_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_SPIRV_LIBS_RELEASE SPIRV)
set(glslang_glslang_SPIRV_SYSTEM_LIBS_RELEASE )
set(glslang_glslang_SPIRV_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_SPIRV_FRAMEWORKS_RELEASE )
set(glslang_glslang_SPIRV_DEPENDENCIES_RELEASE glslang::glslang SPIRV-Tools-opt)
set(glslang_glslang_SPIRV_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_SPIRV_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_SPIRV_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_SPIRV_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_SPIRV_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_SPIRV_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_SPIRV_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_SPIRV_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_SPIRV_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_SPIRV_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::glslang VARIABLES ############################################

set(glslang_glslang_glslang_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_glslang_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_glslang_BIN_DIRS_RELEASE )
set(glslang_glslang_glslang_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_glslang_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_glslang_RES_DIRS_RELEASE )
set(glslang_glslang_glslang_DEFINITIONS_RELEASE )
set(glslang_glslang_glslang_OBJECTS_RELEASE )
set(glslang_glslang_glslang_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_glslang_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_glslang_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_glslang_LIBS_RELEASE glslang)
set(glslang_glslang_glslang_SYSTEM_LIBS_RELEASE m pthread)
set(glslang_glslang_glslang_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_glslang_FRAMEWORKS_RELEASE )
set(glslang_glslang_glslang_DEPENDENCIES_RELEASE glslang::MachineIndependent glslang::GenericCodeGen glslang::OSDependent)
set(glslang_glslang_glslang_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_glslang_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_glslang_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_glslang_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_glslang_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_glslang_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_glslang_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_glslang_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_glslang_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_glslang_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::MachineIndependent VARIABLES ############################################

set(glslang_glslang_MachineIndependent_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_MachineIndependent_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_MachineIndependent_BIN_DIRS_RELEASE )
set(glslang_glslang_MachineIndependent_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_MachineIndependent_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_MachineIndependent_RES_DIRS_RELEASE )
set(glslang_glslang_MachineIndependent_DEFINITIONS_RELEASE )
set(glslang_glslang_MachineIndependent_OBJECTS_RELEASE )
set(glslang_glslang_MachineIndependent_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_MachineIndependent_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_MachineIndependent_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_MachineIndependent_LIBS_RELEASE MachineIndependent)
set(glslang_glslang_MachineIndependent_SYSTEM_LIBS_RELEASE )
set(glslang_glslang_MachineIndependent_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_MachineIndependent_FRAMEWORKS_RELEASE )
set(glslang_glslang_MachineIndependent_DEPENDENCIES_RELEASE glslang::GenericCodeGen glslang::OSDependent)
set(glslang_glslang_MachineIndependent_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_MachineIndependent_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_MachineIndependent_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_MachineIndependent_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_MachineIndependent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_MachineIndependent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_MachineIndependent_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_MachineIndependent_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_MachineIndependent_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_MachineIndependent_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::glslang-default-resource-limits VARIABLES ############################################

set(glslang_glslang_glslang-default-resource-limits_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_glslang-default-resource-limits_BIN_DIRS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_glslang-default-resource-limits_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_glslang-default-resource-limits_RES_DIRS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_DEFINITIONS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_OBJECTS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_glslang-default-resource-limits_LIBS_RELEASE glslang-default-resource-limits)
set(glslang_glslang_glslang-default-resource-limits_SYSTEM_LIBS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_FRAMEWORKS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_DEPENDENCIES_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_glslang-default-resource-limits_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_glslang-default-resource-limits_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_glslang-default-resource-limits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_glslang-default-resource-limits_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_glslang-default-resource-limits_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_glslang-default-resource-limits_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::SPVRemapper VARIABLES ############################################

set(glslang_glslang_SPVRemapper_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_SPVRemapper_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_SPVRemapper_BIN_DIRS_RELEASE )
set(glslang_glslang_SPVRemapper_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_SPVRemapper_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_SPVRemapper_RES_DIRS_RELEASE )
set(glslang_glslang_SPVRemapper_DEFINITIONS_RELEASE )
set(glslang_glslang_SPVRemapper_OBJECTS_RELEASE )
set(glslang_glslang_SPVRemapper_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_SPVRemapper_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_SPVRemapper_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_SPVRemapper_LIBS_RELEASE SPVRemapper)
set(glslang_glslang_SPVRemapper_SYSTEM_LIBS_RELEASE )
set(glslang_glslang_SPVRemapper_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_SPVRemapper_FRAMEWORKS_RELEASE )
set(glslang_glslang_SPVRemapper_DEPENDENCIES_RELEASE )
set(glslang_glslang_SPVRemapper_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_SPVRemapper_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_SPVRemapper_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_SPVRemapper_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_SPVRemapper_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_SPVRemapper_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_SPVRemapper_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_SPVRemapper_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_SPVRemapper_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_SPVRemapper_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::OSDependent VARIABLES ############################################

set(glslang_glslang_OSDependent_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_OSDependent_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_OSDependent_BIN_DIRS_RELEASE )
set(glslang_glslang_OSDependent_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_OSDependent_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_OSDependent_RES_DIRS_RELEASE )
set(glslang_glslang_OSDependent_DEFINITIONS_RELEASE )
set(glslang_glslang_OSDependent_OBJECTS_RELEASE )
set(glslang_glslang_OSDependent_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_OSDependent_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_OSDependent_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_OSDependent_LIBS_RELEASE OSDependent)
set(glslang_glslang_OSDependent_SYSTEM_LIBS_RELEASE pthread)
set(glslang_glslang_OSDependent_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_OSDependent_FRAMEWORKS_RELEASE )
set(glslang_glslang_OSDependent_DEPENDENCIES_RELEASE )
set(glslang_glslang_OSDependent_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_OSDependent_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_OSDependent_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_OSDependent_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_OSDependent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_OSDependent_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_OSDependent_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_OSDependent_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_OSDependent_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_OSDependent_COMPILE_OPTIONS_C_RELEASE}>")
########### COMPONENT glslang::GenericCodeGen VARIABLES ############################################

set(glslang_glslang_GenericCodeGen_INCLUDE_DIRS_RELEASE )
set(glslang_glslang_GenericCodeGen_LIB_DIRS_RELEASE "${glslang_PACKAGE_FOLDER_RELEASE}/lib")
set(glslang_glslang_GenericCodeGen_BIN_DIRS_RELEASE )
set(glslang_glslang_GenericCodeGen_LIBRARY_TYPE_RELEASE STATIC)
set(glslang_glslang_GenericCodeGen_IS_HOST_WINDOWS_RELEASE 0)
set(glslang_glslang_GenericCodeGen_RES_DIRS_RELEASE )
set(glslang_glslang_GenericCodeGen_DEFINITIONS_RELEASE )
set(glslang_glslang_GenericCodeGen_OBJECTS_RELEASE )
set(glslang_glslang_GenericCodeGen_COMPILE_DEFINITIONS_RELEASE )
set(glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_C_RELEASE "")
set(glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_CXX_RELEASE "")
set(glslang_glslang_GenericCodeGen_LIBS_RELEASE GenericCodeGen)
set(glslang_glslang_GenericCodeGen_SYSTEM_LIBS_RELEASE )
set(glslang_glslang_GenericCodeGen_FRAMEWORK_DIRS_RELEASE )
set(glslang_glslang_GenericCodeGen_FRAMEWORKS_RELEASE )
set(glslang_glslang_GenericCodeGen_DEPENDENCIES_RELEASE )
set(glslang_glslang_GenericCodeGen_SHARED_LINK_FLAGS_RELEASE )
set(glslang_glslang_GenericCodeGen_EXE_LINK_FLAGS_RELEASE )
set(glslang_glslang_GenericCodeGen_NO_SONAME_MODE_RELEASE FALSE)

# COMPOUND VARIABLES
set(glslang_glslang_GenericCodeGen_LINKER_FLAGS_RELEASE
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${glslang_glslang_GenericCodeGen_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${glslang_glslang_GenericCodeGen_SHARED_LINK_FLAGS_RELEASE}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${glslang_glslang_GenericCodeGen_EXE_LINK_FLAGS_RELEASE}>
)
set(glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${glslang_glslang_GenericCodeGen_COMPILE_OPTIONS_C_RELEASE}>")