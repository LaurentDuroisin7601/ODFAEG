# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/imgui-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${imgui_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${imgui_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET imgui::imgui)
    add_library(imgui::imgui INTERFACE IMPORTED)
    message(${imgui_MESSAGE_MODE} "Conan: Target declared 'imgui::imgui'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/imgui-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()