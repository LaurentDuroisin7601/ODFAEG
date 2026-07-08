# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/EnTT-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${entt_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${EnTT_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET EnTT::EnTT)
    add_library(EnTT::EnTT INTERFACE IMPORTED)
    message(${EnTT_MESSAGE_MODE} "Conan: Target declared 'EnTT::EnTT'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/EnTT-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()