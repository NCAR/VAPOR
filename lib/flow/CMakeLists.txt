set (SRC
	Particle.cpp
	OceanField.cpp
	SteadyVAPORVelocity.cpp
	SteadyVAPORScalar.cpp
	UnsteadyVAPORField.cpp
	Advection.cpp
	Field.cpp
	VelocityField.cpp
	ScalarField.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/Advection.h
	${PROJECT_SOURCE_DIR}/include/vapor/OceanField.h
	${PROJECT_SOURCE_DIR}/include/vapor/Particle.h
	${PROJECT_SOURCE_DIR}/include/vapor/SteadyVAPORVelocity.h
	${PROJECT_SOURCE_DIR}/include/vapor/SteadyVAPORScalar.h
	${PROJECT_SOURCE_DIR}/include/vapor/UnsteadyVAPORField.h
	${PROJECT_SOURCE_DIR}/include/vapor/Field.h
	${PROJECT_SOURCE_DIR}/include/vapor/VelocityField.h
	${PROJECT_SOURCE_DIR}/include/vapor/ScalarField.h
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
