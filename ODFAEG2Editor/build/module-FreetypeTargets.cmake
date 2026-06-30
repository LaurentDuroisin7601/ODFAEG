# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/module-Freetype-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${freetype_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${Freetype_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype INTERFACE IMPORTED)
    message(${Freetype_MESSAGE_MODE} "Conan: Target declared 'Freetype::Freetype'")
endif()
if(NOT TARGET freetype)
    add_library(freetype INTERFACE IMPORTED)
    set_property(TARGET freetype PROPERTY INTERFACE_LINK_LIBRARIES Freetype::Freetype)
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/module-Freetype-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()