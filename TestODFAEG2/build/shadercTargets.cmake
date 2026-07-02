# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/shaderc-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${shaderc_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${shaderc_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET shaderc::shaderc)
    add_library(shaderc::shaderc INTERFACE IMPORTED)
    message(${shaderc_MESSAGE_MODE} "Conan: Target declared 'shaderc::shaderc'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/shaderc-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()