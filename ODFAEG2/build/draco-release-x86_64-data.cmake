########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(draco_COMPONENT_NAMES "")
if(DEFINED draco_FIND_DEPENDENCY_NAMES)
  list(APPEND draco_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES draco_FIND_DEPENDENCY_NAMES)
else()
  set(draco_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(draco_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/dracoda068cd4a19f1/p")
set(draco_BUILD_MODULES_PATHS_RELEASE )


set(draco_INCLUDE_DIRS_RELEASE )
set(draco_RES_DIRS_RELEASE )
set(draco_DEFINITIONS_RELEASE )
set(draco_SHARED_LINK_FLAGS_RELEASE )
set(draco_EXE_LINK_FLAGS_RELEASE )
set(draco_OBJECTS_RELEASE )
set(draco_COMPILE_DEFINITIONS_RELEASE )
set(draco_COMPILE_OPTIONS_C_RELEASE )
set(draco_COMPILE_OPTIONS_CXX_RELEASE )
set(draco_LIB_DIRS_RELEASE "${draco_PACKAGE_FOLDER_RELEASE}/lib")
set(draco_BIN_DIRS_RELEASE )
set(draco_LIBRARY_TYPE_RELEASE STATIC)
set(draco_IS_HOST_WINDOWS_RELEASE 0)
set(draco_LIBS_RELEASE draco)
set(draco_SYSTEM_LIBS_RELEASE m)
set(draco_FRAMEWORK_DIRS_RELEASE )
set(draco_FRAMEWORKS_RELEASE )
set(draco_BUILD_DIRS_RELEASE )
set(draco_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(draco_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${draco_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${draco_COMPILE_OPTIONS_C_RELEASE}>")
set(draco_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${draco_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${draco_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${draco_EXE_LINK_FLAGS_RELEASE}>")


set(draco_COMPONENTS_RELEASE )