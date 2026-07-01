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
set(assimp_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/assimceabed90671bd/p")
set(assimp_BUILD_MODULES_PATHS_DEBUG )


set(assimp_INCLUDE_DIRS_DEBUG "${assimp_PACKAGE_FOLDER_DEBUG}/include")
set(assimp_RES_DIRS_DEBUG )
set(assimp_DEFINITIONS_DEBUG )
set(assimp_SHARED_LINK_FLAGS_DEBUG )
set(assimp_EXE_LINK_FLAGS_DEBUG )
set(assimp_OBJECTS_DEBUG )
set(assimp_COMPILE_DEFINITIONS_DEBUG )
set(assimp_COMPILE_OPTIONS_C_DEBUG )
set(assimp_COMPILE_OPTIONS_CXX_DEBUG )
set(assimp_LIB_DIRS_DEBUG "${assimp_PACKAGE_FOLDER_DEBUG}/lib")
set(assimp_BIN_DIRS_DEBUG )
set(assimp_LIBRARY_TYPE_DEBUG STATIC)
set(assimp_IS_HOST_WINDOWS_DEBUG 0)
set(assimp_LIBS_DEBUG assimp)
set(assimp_SYSTEM_LIBS_DEBUG rt m pthread stdc++)
set(assimp_FRAMEWORK_DIRS_DEBUG )
set(assimp_FRAMEWORKS_DEBUG )
set(assimp_BUILD_DIRS_DEBUG )
set(assimp_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(assimp_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${assimp_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${assimp_COMPILE_OPTIONS_C_DEBUG}>")
set(assimp_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${assimp_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${assimp_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${assimp_EXE_LINK_FLAGS_DEBUG}>")


set(assimp_COMPONENTS_DEBUG )