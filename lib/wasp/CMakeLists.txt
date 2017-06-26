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


target_compile_options(wasp PRIVATE "-Wno-deprecated-register")
target_compile_options(wasp PRIVATE "-Wno-absolute-value")
