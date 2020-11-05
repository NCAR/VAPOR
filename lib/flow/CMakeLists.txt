set (SRC
	Particle.cpp
	Advection.cpp
	Field.cpp
	VaporField.cpp
    GrownGrid.cpp
    AdvectionIO.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/Advection.h
	${PROJECT_SOURCE_DIR}/include/vapor/Particle.h
	${PROJECT_SOURCE_DIR}/include/vapor/Field.h
	${PROJECT_SOURCE_DIR}/include/vapor/VaporField.h
	${PROJECT_SOURCE_DIR}/include/vapor/AdvectionIO.h
    GrownGrid.h
)

add_library (flow SHARED ${SRC} ${HEADERS})

target_link_libraries ( flow  PUBLIC vdc params )

add_definitions (-DFLOW_EXPORTS)

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
