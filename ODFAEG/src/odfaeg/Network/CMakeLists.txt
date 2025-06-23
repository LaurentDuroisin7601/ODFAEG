include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Network)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Network)

set(SRC
	${INCROOT}/aes.h
	${INCROOT}/export.hpp
	${SRCROOT}/aes.cpp
	${INCROOT}/rsa.h
	${SRCROOT}/rsa.cpp
	${INCROOT}/encryptedPacket.h
	${SRCROOT}/encryptedPacket.cpp
	${INCROOT}/symEncPacket.h
	${SRCROOT}/symEncPacket.cpp
	${INCROOT}/client.h
	${SRCROOT}/client.cpp
	${INCROOT}/srkserveur.h
	${SRCROOT}/srkserveur.cpp
	${INCROOT}/network.h
	${SRCROOT}/network.cpp
	${INCROOT}/user.h
	${SRCROOT}/user.cpp	
	${INCROOT}/cliEncryptedPacket.hpp
	${SRCROOT}/cliEncryptedPacket.cpp
	${INCROOT}/application.h
	${SRCROOT}/application.cpp
	${INCROOT}/ECS/application.hpp
	${SRCROOT}/ECS/application.cpp
	${INCROOT}/packet.hpp
	${SRCROOT}/packet.cpp
	${INCROOT}/socket.hpp
	${SRCROOT}/socket.cpp
	${INCROOT}/http.hpp
	${SRCROOT}/http.cpp
	${INCROOT}/ipaddress.hpp
	${SRCROOT}/ipaddress.cpp
	${INCROOT}/socketselector.hpp
	${SRCROOT}/socketselector.cpp
	${INCROOT}/tcpsocket.hpp
	${SRCROOT}/tcpsocket.cpp
	${INCROOT}/tcplistener.hpp
	${SRCROOT}/tcplistener.cpp
	${INCROOT}/updsocket.hpp
	${SRCROOT}/updsocket.cpp
	)
        if (WIN32)
        set(PLATEFORM_SRC
        ${INCROOT}/Windows/socketImpl.hpp
	${SRCROOT}/Windows/socketImpl.cpp)
        endif()	
#find_package (OpenSSL 1 REQUIRED)
include_directories(${CMAKE_INCLUDE_PATH} ${OPENSSL_INCLUDE_DIR})
link_directories(${CMAKE_LIBRARY_PATH})
sfgl_add_library(odfaeg-network
			 SOURCES ${SRC} ${PLATEFORM_SRC})
target_link_libraries (odfaeg-network odfaeg-graphics odfaeg-window odfaeg-physics odfaeg-math odfaeg-core sfml-network sfml-audio sfml-graphics sfml-window sfml-system libssl libcrypto)
