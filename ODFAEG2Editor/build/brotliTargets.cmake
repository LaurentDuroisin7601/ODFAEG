# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/brotli-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${brotli_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${brotli_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET brotli::brotli)
    add_library(brotli::brotli INTERFACE IMPORTED)
    message(${brotli_MESSAGE_MODE} "Conan: Target declared 'brotli::brotli'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/brotli-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()