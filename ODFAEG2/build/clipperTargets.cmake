# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/clipper-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${clipper_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${clipper_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET clipper::clipper)
    add_library(clipper::clipper INTERFACE IMPORTED)
    message(${clipper_MESSAGE_MODE} "Conan: Target declared 'clipper::clipper'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/clipper-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()