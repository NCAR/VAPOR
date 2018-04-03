configure_file (CMakeConfig.cpp.in CMakeConfig.cpp)

add_library (common SHARED
	common.cpp
	MyBase.cpp
	OptionParser.cpp
	EasyThreads.cpp
	CFuncs.cpp
	Version.cpp
	PVTime.cpp
	GetAppPath.cpp
	utils.cpp
	${CMAKE_CURRENT_BINARY_DIR}/CMakeConfig.cpp
	)

target_link_libraries (common pthread)

if (APPLE)
	find_library (COREFOUNDATION CoreFoundation)
	target_link_libraries (common ${COREFOUNDATION})
endif()

if (NOT WIN32)
	add_definitions (-DENABLE_THREADS)
endif()

if (NOT WIN32)
	target_compile_options(common PRIVATE "-Wno-deprecated-declarations")
endif ()

add_definitions (-DCOMMON_EXPORTS)

install (
	TARGETS common
	DESTINATION ${INSTALL_LIB_DIR}
	COMPONENT Libraries
	)

# Not needed since vapor shares include directory
# target_include_directories (common PUBLIC ${VAPOR_SOURCE_DIR}/include)
