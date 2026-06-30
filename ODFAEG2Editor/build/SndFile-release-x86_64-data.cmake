########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libsndfile_COMPONENT_NAMES "")
if(DEFINED libsndfile_FIND_DEPENDENCY_NAMES)
  list(APPEND libsndfile_FIND_DEPENDENCY_NAMES Vorbis flac Ogg Opus mpg123 libmp3lame)
  list(REMOVE_DUPLICATES libsndfile_FIND_DEPENDENCY_NAMES)
else()
  set(libsndfile_FIND_DEPENDENCY_NAMES Vorbis flac Ogg Opus mpg123 libmp3lame)
endif()
set(Vorbis_FIND_MODE "NO_MODULE")
set(flac_FIND_MODE "NO_MODULE")
set(Ogg_FIND_MODE "NO_MODULE")
set(Opus_FIND_MODE "NO_MODULE")
set(mpg123_FIND_MODE "NO_MODULE")
set(libmp3lame_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libsndfile_PACKAGE_FOLDER_RELEASE "/home/laurent/.conan2/p/b/libsnfc3277acb96c2/p")
set(libsndfile_BUILD_MODULES_PATHS_RELEASE )


set(libsndfile_INCLUDE_DIRS_RELEASE "${libsndfile_PACKAGE_FOLDER_RELEASE}/include")
set(libsndfile_RES_DIRS_RELEASE )
set(libsndfile_DEFINITIONS_RELEASE )
set(libsndfile_SHARED_LINK_FLAGS_RELEASE )
set(libsndfile_EXE_LINK_FLAGS_RELEASE )
set(libsndfile_OBJECTS_RELEASE )
set(libsndfile_COMPILE_DEFINITIONS_RELEASE )
set(libsndfile_COMPILE_OPTIONS_C_RELEASE )
set(libsndfile_COMPILE_OPTIONS_CXX_RELEASE )
set(libsndfile_LIB_DIRS_RELEASE "${libsndfile_PACKAGE_FOLDER_RELEASE}/lib")
set(libsndfile_BIN_DIRS_RELEASE )
set(libsndfile_LIBRARY_TYPE_RELEASE STATIC)
set(libsndfile_IS_HOST_WINDOWS_RELEASE 0)
set(libsndfile_LIBS_RELEASE sndfile)
set(libsndfile_SYSTEM_LIBS_RELEASE m dl pthread rt)
set(libsndfile_FRAMEWORK_DIRS_RELEASE )
set(libsndfile_FRAMEWORKS_RELEASE )
set(libsndfile_BUILD_DIRS_RELEASE )
set(libsndfile_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(libsndfile_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${libsndfile_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${libsndfile_COMPILE_OPTIONS_C_RELEASE}>")
set(libsndfile_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libsndfile_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libsndfile_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libsndfile_EXE_LINK_FLAGS_RELEASE}>")


set(libsndfile_COMPONENTS_RELEASE )