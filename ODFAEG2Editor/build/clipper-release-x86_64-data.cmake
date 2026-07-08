########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(clipper_COMPONENT_NAMES "")
if(DEFINED clipper_FIND_DEPENDENCY_NAMES)
  list(APPEND clipper_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES clipper_FIND_DEPENDENCY_NAMES)
else()
  set(clipper_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(clipper_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/clippded9a3338d852/p")
set(clipper_BUILD_MODULES_PATHS_RELEASE )


set(clipper_INCLUDE_DIRS_RELEASE )
set(clipper_RES_DIRS_RELEASE )
set(clipper_DEFINITIONS_RELEASE )
set(clipper_SHARED_LINK_FLAGS_RELEASE )
set(clipper_EXE_LINK_FLAGS_RELEASE )
set(clipper_OBJECTS_RELEASE )
set(clipper_COMPILE_DEFINITIONS_RELEASE )
set(clipper_COMPILE_OPTIONS_C_RELEASE )
set(clipper_COMPILE_OPTIONS_CXX_RELEASE )
set(clipper_LIB_DIRS_RELEASE "${clipper_PACKAGE_FOLDER_RELEASE}/lib")
set(clipper_BIN_DIRS_RELEASE )
set(clipper_LIBRARY_TYPE_RELEASE STATIC)
set(clipper_IS_HOST_WINDOWS_RELEASE 0)
set(clipper_LIBS_RELEASE polyclipping)
set(clipper_SYSTEM_LIBS_RELEASE m)
set(clipper_FRAMEWORK_DIRS_RELEASE )
set(clipper_FRAMEWORKS_RELEASE )
set(clipper_BUILD_DIRS_RELEASE )
set(clipper_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(clipper_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${clipper_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${clipper_COMPILE_OPTIONS_C_RELEASE}>")
set(clipper_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${clipper_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${clipper_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${clipper_EXE_LINK_FLAGS_RELEASE}>")


set(clipper_COMPONENTS_RELEASE )