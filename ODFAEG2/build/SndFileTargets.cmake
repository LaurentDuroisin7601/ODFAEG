# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/SndFile-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libsndfile_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${SndFile_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET SndFile::sndfile)
    add_library(SndFile::sndfile INTERFACE IMPORTED)
    message(${SndFile_MESSAGE_MODE} "Conan: Target declared 'SndFile::sndfile'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/SndFile-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()