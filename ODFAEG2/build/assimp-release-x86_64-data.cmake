########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(assimp_COMPONENT_NAMES "")
if(DEFINED assimp_FIND_DEPENDENCY_NAMES)
  list(APPEND assimp_FIND_DEPENDENCY_NAMES minizip pugixml zip poly2tri draco clipper openddlparser ZLIB stb)
  list(REMOVE_DUPLICATES assimp_FIND_DEPENDENCY_NAMES)
else()
  set(assimp_FIND_DEPENDENCY_NAMES minizip pugixml zip poly2tri draco clipper openddlparser ZLIB stb)
endif()
set(minizip_FIND_MODE "NO_MODULE")
set(pugixml_FIND_MODE "NO_MODULE")
set(zip_FIND_MODE "NO_MODULE")
set(poly2tri_FIND_MODE "NO_MODULE")
set(draco_FIND_MODE "NO_MODULE")
set(clipper_FIND_MODE "NO_MODULE")
set(openddlparser_FIND_MODE "NO_MODULE")
set(ZLIB_FIND_MODE "NO_MODULE")
set(stb_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(assimp_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/assim1430cbbc53906/p")
set(assimp_BUILD_MODULES_PATHS_RELEASE )


set(assimp_INCLUDE_DIRS_RELEASE "${assimp_PACKAGE_FOLDER_RELEASE}/include")
set(assimp_RES_DIRS_RELEASE )
set(assimp_DEFINITIONS_RELEASE )
set(assimp_SHARED_LINK_FLAGS_RELEASE )
set(assimp_EXE_LINK_FLAGS_RELEASE )
set(assimp_OBJECTS_RELEASE )
set(assimp_COMPILE_DEFINITIONS_RELEASE )
set(assimp_COMPILE_OPTIONS_C_RELEASE )
set(assimp_COMPILE_OPTIONS_CXX_RELEASE )
set(assimp_LIB_DIRS_RELEASE "${assimp_PACKAGE_FOLDER_RELEASE}/lib")
set(assimp_BIN_DIRS_RELEASE )
set(assimp_LIBRARY_TYPE_RELEASE STATIC)
set(assimp_IS_HOST_WINDOWS_RELEASE 0)
set(assimp_LIBS_RELEASE assimp)
set(assimp_SYSTEM_LIBS_RELEASE rt m pthread stdc++)
set(assimp_FRAMEWORK_DIRS_RELEASE )
set(assimp_FRAMEWORKS_RELEASE )
set(assimp_BUILD_DIRS_RELEASE )
set(assimp_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(assimp_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${assimp_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${assimp_COMPILE_OPTIONS_C_RELEASE}>")
set(assimp_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${assimp_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${assimp_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${assimp_EXE_LINK_FLAGS_RELEASE}>")


set(assimp_COMPONENTS_RELEASE )