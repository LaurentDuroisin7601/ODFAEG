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
set(libsndfile_PACKAGE_FOLDER_DEBUG "/home/laurent/.conan2/p/b/libsnaa3d5a61b969a/p")
set(libsndfile_BUILD_MODULES_PATHS_DEBUG )


set(libsndfile_INCLUDE_DIRS_DEBUG "${libsndfile_PACKAGE_FOLDER_DEBUG}/include")
set(libsndfile_RES_DIRS_DEBUG )
set(libsndfile_DEFINITIONS_DEBUG )
set(libsndfile_SHARED_LINK_FLAGS_DEBUG )
set(libsndfile_EXE_LINK_FLAGS_DEBUG )
set(libsndfile_OBJECTS_DEBUG )
set(libsndfile_COMPILE_DEFINITIONS_DEBUG )
set(libsndfile_COMPILE_OPTIONS_C_DEBUG )
set(libsndfile_COMPILE_OPTIONS_CXX_DEBUG )
set(libsndfile_LIB_DIRS_DEBUG "${libsndfile_PACKAGE_FOLDER_DEBUG}/lib")
set(libsndfile_BIN_DIRS_DEBUG )
set(libsndfile_LIBRARY_TYPE_DEBUG STATIC)
set(libsndfile_IS_HOST_WINDOWS_DEBUG 0)
set(libsndfile_LIBS_DEBUG sndfile)
set(libsndfile_SYSTEM_LIBS_DEBUG m dl pthread rt)
set(libsndfile_FRAMEWORK_DIRS_DEBUG )
set(libsndfile_FRAMEWORKS_DEBUG )
set(libsndfile_BUILD_DIRS_DEBUG )
set(libsndfile_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libsndfile_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libsndfile_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libsndfile_COMPILE_OPTIONS_C_DEBUG}>")
set(libsndfile_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libsndfile_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libsndfile_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libsndfile_EXE_LINK_FLAGS_DEBUG}>")


set(libsndfile_COMPONENTS_DEBUG )