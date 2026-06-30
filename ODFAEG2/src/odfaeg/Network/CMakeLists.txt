set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Network)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
	"${SRCROOT}/*.cpp"
    	"${INCROOT}/*.hpp"
)
add_library(odfaeg-network STATIC ${SRC})
target_include_directories(odfaeg-network PUBLIC
    ${INCROOT}
)
target_link_libraries(odfaeg-network PRIVATE odfaeg-math odfaeg-core)
find_package(OpenSSL REQUIRED)

target_link_libraries(odfaeg-network PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)
target_link_libraries(odfaeg-network PRIVATE ws2_32)
install(TARGETS odfaeg-network
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Installer les headers
install(DIRECTORY ${INCROOT}/
    DESTINATION include/odfaeg/Network
    FILES_MATCHING PATTERN "*.hpp"
)