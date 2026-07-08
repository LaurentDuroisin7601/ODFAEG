########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(shaderc_COMPONENT_NAMES "")
if(DEFINED shaderc_FIND_DEPENDENCY_NAMES)
  list(APPEND shaderc_FIND_DEPENDENCY_NAMES glslang SPIRV-Tools)
  list(REMOVE_DUPLICATES shaderc_FIND_DEPENDENCY_NAMES)
else()
  set(shaderc_FIND_DEPENDENCY_NAMES glslang SPIRV-Tools)
endif()
set(glslang_FIND_MODE "NO_MODULE")
set(SPIRV-Tools_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(shaderc_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/shade666a6e1056262/p")
set(shaderc_BUILD_MODULES_PATHS_RELEASE )


set(shaderc_INCLUDE_DIRS_RELEASE "${shaderc_PACKAGE_FOLDER_RELEASE}/include")
set(shaderc_RES_DIRS_RELEASE )
set(shaderc_DEFINITIONS_RELEASE )
set(shaderc_SHARED_LINK_FLAGS_RELEASE )
set(shaderc_EXE_LINK_FLAGS_RELEASE )
set(shaderc_OBJECTS_RELEASE )
set(shaderc_COMPILE_DEFINITIONS_RELEASE )
set(shaderc_COMPILE_OPTIONS_C_RELEASE )
set(shaderc_COMPILE_OPTIONS_CXX_RELEASE )
set(shaderc_LIB_DIRS_RELEASE "${shaderc_PACKAGE_FOLDER_RELEASE}/lib")
set(shaderc_BIN_DIRS_RELEASE )
set(shaderc_LIBRARY_TYPE_RELEASE STATIC)
set(shaderc_IS_HOST_WINDOWS_RELEASE 0)
set(shaderc_LIBS_RELEASE shaderc shaderc_util)
set(shaderc_SYSTEM_LIBS_RELEASE stdc++ pthread)
set(shaderc_FRAMEWORK_DIRS_RELEASE )
set(shaderc_FRAMEWORKS_RELEASE )
set(shaderc_BUILD_DIRS_RELEASE )
set(shaderc_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(shaderc_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${shaderc_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${shaderc_COMPILE_OPTIONS_C_RELEASE}>")
set(shaderc_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${shaderc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${shaderc_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${shaderc_EXE_LINK_FLAGS_RELEASE}>")


set(shaderc_COMPONENTS_RELEASE )