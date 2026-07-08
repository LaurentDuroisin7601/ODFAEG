# Conan CMake Provider
include(FetchContent)

set(CONAN_PROVIDER_VERSION 0.2)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    message(STATUS "Conan: Installing dependencies")
    execute_process(
        COMMAND conan install ${CMAKE_SOURCE_DIR}
                --output-folder=${CMAKE_BINARY_DIR}o

                --build=missing
        RESULT_VARIABLE result
    )
    if(result)
        message(FATAL_ERROR "Conan install failed: ${result}")
    endif()
endif()

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)
