# include the ODFAEG specific macros
include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

set(INCROOT ${PROJECT_SOURCE_DIR}/include/odfaeg/Physics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/odfaeg/Physics)
set(SRC
	
	${INCROOT}/export.hpp
	${INCROOT}/collisionResultSet.hpp
	${SRCROOT}/collisionResultSet.cpp
	${INCROOT}/boundingVolume.h
	${INCROOT}/boundingSphere.h
	${SRCROOT}/boundingSphere.cpp
	${INCROOT}/boundingEllipsoid.h
	${SRCROOT}/boundingEllipsoid.cpp
	${INCROOT}/boundingBox.h
	${SRCROOT}/boundingBox.cpp
	${INCROOT}/orientedBoundingBox.h
	${SRCROOT}/orientedBoundingBox.cpp
	${INCROOT}/boundingPolyhedron.h
	${SRCROOT}/boundingPolyhedron.cpp
	${INCROOT}/affector.h
	${SRCROOT}/affector.cpp
	${INCROOT}/emmiters.h
	${INCROOT}/emissionInterface.h
	${SRCROOT}/emitter.cpp
	${INCROOT}/particule.h
        ${SRCROOT}/particle.cpp        
	${INCROOT}/boundingCircle.hpp
	${SRCROOT}/boundingCircle.cpp)
	
include_directories(${CMAKE_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})
#find_package (SFML 2 COMPONENTS window system graphics audio REQUIRED)
sfgl_add_library(odfaeg-physics
				 SOURCES ${SRC})
target_link_libraries (odfaeg-physics odfaeg-math odfaeg-core)
	
