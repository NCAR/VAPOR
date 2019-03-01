set (SRC
	Particle.cpp
	VelocityField.cpp
	OceanField.cpp
	SteadyVAPORField.cpp
	UnsteadyVAPORField.cpp
	Advection.cpp
)

set (HEADERS
	Advection.h
	OceanField.h
	Particle.h
	SteadyVAPORField.h
	UnsteadyVAPORField.h
	VelocityField.h
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
