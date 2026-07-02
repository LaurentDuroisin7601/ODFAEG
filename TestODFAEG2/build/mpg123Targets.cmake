# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/mpg123-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${mpg123_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${mpg123_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET mpg123::mpg123)
    add_library(mpg123::mpg123 INTERFACE IMPORTED)
    message(${mpg123_MESSAGE_MODE} "Conan: Target declared 'mpg123::mpg123'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/mpg123-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()