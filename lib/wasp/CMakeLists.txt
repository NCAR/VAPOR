add_library (wasp SHARED
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

target_link_libraries (wasp common netcdf)


if (NOT WIN32)
	target_compile_options(wasp PRIVATE "-Wno-deprecated-register")
	target_compile_options(wasp PRIVATE "-Wno-absolute-value")
endif ()

add_definitions (-DWASP_EXPORTS)
