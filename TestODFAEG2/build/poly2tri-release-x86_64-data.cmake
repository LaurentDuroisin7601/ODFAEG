########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(poly2tri_COMPONENT_NAMES "")
if(DEFINED poly2tri_FIND_DEPENDENCY_NAMES)
  list(APPEND poly2tri_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES poly2tri_FIND_DEPENDENCY_NAMES)
else()
  set(poly2tri_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(poly2tri_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/poly25edfc427eed4d/p")
set(poly2tri_BUILD_MODULES_PATHS_RELEASE )


set(poly2tri_INCLUDE_DIRS_RELEASE )
set(poly2tri_RES_DIRS_RELEASE )
set(poly2tri_DEFINITIONS_RELEASE )
set(poly2tri_SHARED_LINK_FLAGS_RELEASE )
set(poly2tri_EXE_LINK_FLAGS_RELEASE )
set(poly2tri_OBJECTS_RELEASE )
set(poly2tri_COMPILE_DEFINITIONS_RELEASE )
set(poly2tri_COMPILE_OPTIONS_C_RELEASE )
set(poly2tri_COMPILE_OPTIONS_CXX_RELEASE )
set(poly2tri_LIB_DIRS_RELEASE "${poly2tri_PACKAGE_FOLDER_RELEASE}/lib")
set(poly2tri_BIN_DIRS_RELEASE )
set(poly2tri_LIBRARY_TYPE_RELEASE STATIC)
set(poly2tri_IS_HOST_WINDOWS_RELEASE 0)
set(poly2tri_LIBS_RELEASE poly2tri)
set(poly2tri_SYSTEM_LIBS_RELEASE m)
set(poly2tri_FRAMEWORK_DIRS_RELEASE )
set(poly2tri_FRAMEWORKS_RELEASE )
set(poly2tri_BUILD_DIRS_RELEASE )
set(poly2tri_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(poly2tri_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${poly2tri_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${poly2tri_COMPILE_OPTIONS_C_RELEASE}>")
set(poly2tri_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${poly2tri_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${poly2tri_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${poly2tri_EXE_LINK_FLAGS_RELEASE}>")


set(poly2tri_COMPONENTS_RELEASE )