set (SRC
	Compressor.cpp
	MatWaveBase.cpp
	MatWaveDwt.cpp
	MatWaveWavedec.cpp
	NetCDFCpp.cpp
	SignificanceMap.cpp
	WASP.cpp
	WaveFiltBase.cpp
	WaveFiltBior.cpp
	WaveFiltCoif.cpp
	WaveFiltDaub.cpp
	WaveFiltHaar.cpp
	WaveFiltInt.cpp
)

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/Compressor.h
	${PROJECT_SOURCE_DIR}/include/vapor/MatWaveBase.h
	${PROJECT_SOURCE_DIR}/include/vapor/MatWaveDwt.h
	${PROJECT_SOURCE_DIR}/include/vapor/MatWaveWavedec.h
	${PROJECT_SOURCE_DIR}/include/vapor/NetCDFCpp.h
	${PROJECT_SOURCE_DIR}/include/vapor/SignificanceMap.h
	${PROJECT_SOURCE_DIR}/include/vapor/WASP.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltBase.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltBior.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltCoif.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltDaub.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltHaar.h
	${PROJECT_SOURCE_DIR}/include/vapor/WaveFiltInt.h
)

add_library (wasp SHARED ${SRC} ${HEADERS})

target_link_libraries (wasp common netcdf)

if (NOT WIN32)
	target_compile_options(wasp PRIVATE "-Wno-deprecated-register")
	target_compile_options(wasp PRIVATE "-Wno-absolute-value")
endif ()

add_definitions (-DWASP_EXPORTS)

install (
	TARGETS wasp
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)
