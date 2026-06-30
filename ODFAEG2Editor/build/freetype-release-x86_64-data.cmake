########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(freetype_COMPONENT_NAMES "")
if(DEFINED freetype_FIND_DEPENDENCY_NAMES)
  list(APPEND freetype_FIND_DEPENDENCY_NAMES PNG brotli ZLIB BZip2)
  list(REMOVE_DUPLICATES freetype_FIND_DEPENDENCY_NAMES)
else()
  set(freetype_FIND_DEPENDENCY_NAMES PNG brotli ZLIB BZip2)
endif()
set(PNG_FIND_MODE "NO_MODULE")
set(brotli_FIND_MODE "NO_MODULE")
set(ZLIB_FIND_MODE "NO_MODULE")
set(BZip2_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(freetype_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/freetc5315ef469cc1/p")
set(freetype_BUILD_MODULES_PATHS_RELEASE "${freetype_PACKAGE_FOLDER_RELEASE}/lib/cmake/conan-official-freetype-variables.cmake")


set(freetype_INCLUDE_DIRS_RELEASE "${freetype_PACKAGE_FOLDER_RELEASE}/include"
			"${freetype_PACKAGE_FOLDER_RELEASE}/include/freetype2")
set(freetype_RES_DIRS_RELEASE )
set(freetype_DEFINITIONS_RELEASE )
set(freetype_SHARED_LINK_FLAGS_RELEASE )
set(freetype_EXE_LINK_FLAGS_RELEASE )
set(freetype_OBJECTS_RELEASE )
set(freetype_COMPILE_DEFINITIONS_RELEASE )
set(freetype_COMPILE_OPTIONS_C_RELEASE )
set(freetype_COMPILE_OPTIONS_CXX_RELEASE )
set(freetype_LIB_DIRS_RELEASE "${freetype_PACKAGE_FOLDER_RELEASE}/lib")
set(freetype_BIN_DIRS_RELEASE )
set(freetype_LIBRARY_TYPE_RELEASE STATIC)
set(freetype_IS_HOST_WINDOWS_RELEASE 0)
set(freetype_LIBS_RELEASE freetype)
set(freetype_SYSTEM_LIBS_RELEASE m)
set(freetype_FRAMEWORK_DIRS_RELEASE )
set(freetype_FRAMEWORKS_RELEASE )
set(freetype_BUILD_DIRS_RELEASE )
set(freetype_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(freetype_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${freetype_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${freetype_COMPILE_OPTIONS_C_RELEASE}>")
set(freetype_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${freetype_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${freetype_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${freetype_EXE_LINK_FLAGS_RELEASE}>")


set(freetype_COMPONENTS_RELEASE )