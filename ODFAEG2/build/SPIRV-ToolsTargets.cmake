# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${spirv-tools_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${SPIRV-Tools_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET spirv-tools::spirv-tools)
    add_library(spirv-tools::spirv-tools INTERFACE IMPORTED)
    message(${SPIRV-Tools_MESSAGE_MODE} "Conan: Target declared 'spirv-tools::spirv-tools'")
endif()
if(NOT TARGET SPIRV-Tools)
    add_library(SPIRV-Tools INTERFACE IMPORTED)
    set_property(TARGET SPIRV-Tools PROPERTY INTERFACE_LINK_LIBRARIES SPIRV-Tools-static)
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/SPIRV-Tools-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()