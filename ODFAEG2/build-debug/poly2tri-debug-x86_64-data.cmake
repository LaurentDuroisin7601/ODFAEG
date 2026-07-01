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
set(poly2tri_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/poly2e0c1f2765e3cb/p")
set(poly2tri_BUILD_MODULES_PATHS_DEBUG )


set(poly2tri_INCLUDE_DIRS_DEBUG )
set(poly2tri_RES_DIRS_DEBUG )
set(poly2tri_DEFINITIONS_DEBUG )
set(poly2tri_SHARED_LINK_FLAGS_DEBUG )
set(poly2tri_EXE_LINK_FLAGS_DEBUG )
set(poly2tri_OBJECTS_DEBUG )
set(poly2tri_COMPILE_DEFINITIONS_DEBUG )
set(poly2tri_COMPILE_OPTIONS_C_DEBUG )
set(poly2tri_COMPILE_OPTIONS_CXX_DEBUG )
set(poly2tri_LIB_DIRS_DEBUG "${poly2tri_PACKAGE_FOLDER_DEBUG}/lib")
set(poly2tri_BIN_DIRS_DEBUG )
set(poly2tri_LIBRARY_TYPE_DEBUG STATIC)
set(poly2tri_IS_HOST_WINDOWS_DEBUG 0)
set(poly2tri_LIBS_DEBUG poly2tri)
set(poly2tri_SYSTEM_LIBS_DEBUG m)
set(poly2tri_FRAMEWORK_DIRS_DEBUG )
set(poly2tri_FRAMEWORKS_DEBUG )
set(poly2tri_BUILD_DIRS_DEBUG )
set(poly2tri_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(poly2tri_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${poly2tri_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${poly2tri_COMPILE_OPTIONS_C_DEBUG}>")
set(poly2tri_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${poly2tri_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${poly2tri_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${poly2tri_EXE_LINK_FLAGS_DEBUG}>")


set(poly2tri_COMPONENTS_DEBUG )