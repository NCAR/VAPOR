set (SRC
	Particle.cpp
	VelocityField.cpp
	OceanField.cpp
	SteadyVAPORField.cpp
	UnsteadyVAPORField.cpp
	Advection.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/Advection.h
	${PROJECT_SOURCE_DIR}/include/vapor/OceanField.h
	${PROJECT_SOURCE_DIR}/include/vapor/Particle.h
	${PROJECT_SOURCE_DIR}/include/vapor/SteadyVAPORField.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnsteadyVAPORField.h
	${PROJECT_SOURCE_DIR}/include/vapor/VelocityField.h
)

add_library (flow SHARED ${SRC} ${HEADERS})

target_link_libraries ( flow  PUBLIC vdc )

install (
	TARGETS flow
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)
