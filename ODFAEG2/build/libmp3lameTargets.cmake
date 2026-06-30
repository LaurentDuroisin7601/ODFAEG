# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/libmp3lame-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libmp3lame_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${libmp3lame_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET libmp3lame::libmp3lame)
    add_library(libmp3lame::libmp3lame INTERFACE IMPORTED)
    message(${libmp3lame_MESSAGE_MODE} "Conan: Target declared 'libmp3lame::libmp3lame'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/libmp3lame-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()