# Load the debug and release variables
file(GLOB DATA_FILES "${CMAKE_CURRENT_LIST_DIR}/openddlparser-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${openddl-parser_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${openddlparser_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET openddlparser::openddlparser)
    add_library(openddlparser::openddlparser INTERFACE IMPORTED)
    message(${openddlparser_MESSAGE_MODE} "Conan: Target declared 'openddlparser::openddlparser'")
endif()
# Load the debug and release library finders
file(GLOB CONFIG_FILES "${CMAKE_CURRENT_LIST_DIR}/openddlparser-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()