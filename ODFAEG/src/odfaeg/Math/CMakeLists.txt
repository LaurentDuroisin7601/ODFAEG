# include the ODFAEG specific macros
include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)


set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Math)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Math)
set(SRC
	${INCROOT}/transformMatrix.h
	${SRCROOT}/transformMatrix.cpp
	${INCROOT}/export.hpp
	${INCROOT}/computer.h
	${SRCROOT}/computer.cpp
	${INCROOT}/maths.h
	${SRCROOT}/maths.cpp
	${INCROOT}/matrix2.h
	${SRCROOT}/matrix2.cpp
	${INCROOT}/matrix3.h
	${SRCROOT}/matrix3.cpp
	${INCROOT}/matrix4.h
	${SRCROOT}/matrix4.cpp	
	${INCROOT}/quaternion.hpp
	${SRCROOT}/quaternion.cpp
	${INCROOT}/vec2f.h
	${SRCROOT}/vec2.cpp
	${INCROOT}/vec4.h
	${SRCROOT}/vec4.cpp
	${INCROOT}/distributions.h
	${INCROOT}/distribution.h
	${SRCROOT}/distribution.cpp
	${INCROOT}/ray.h
	${SRCROOT}/ray.cpp
	${INCROOT}/bigInt.hpp
	${SRCROOT}/bigInt.cpp)
	
include_directories(${CMAKE_INCLUDE_PATH})
link_directories (${CMAKE_LIBRARY_PATH})
#find_package (SFML 2.5 COMPONENTS window system graphics audio REQUIRED)
sfgl_add_library(odfaeg-math
				 SOURCES ${SRC})
target_link_libraries (odfaeg-math odfaeg-core)

