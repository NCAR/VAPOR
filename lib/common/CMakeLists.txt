configure_file (CMakeConfig.cpp.in CMakeConfig.cpp)

set (SRC
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

set (HEADERS
	${PROJECT_SOURCE_DIR}/include/vapor/common.h
	${PROJECT_SOURCE_DIR}/include/vapor/MyBase.h
	${PROJECT_SOURCE_DIR}/include/vapor/OptionParser.h
	${PROJECT_SOURCE_DIR}/include/vapor/EasyThreads.h
	${PROJECT_SOURCE_DIR}/include/vapor/CFuncs.h
	${PROJECT_SOURCE_DIR}/include/vapor/Version.h
	${PROJECT_SOURCE_DIR}/include/vapor/PVTime.h
	${PROJECT_SOURCE_DIR}/include/vapor/GetAppPath.h
	${PROJECT_SOURCE_DIR}/include/vapor/utils.h
	${PROJECT_SOURCE_DIR}/include/vapor/CMakeConfig.h
)

add_library (common SHARED ${SRC} ${HEADERS})

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

install (
	FILES ${HEADERS}
	DESTINATION ${INSTALL_INCLUDE_DIR}
	COMPONENT Libraries
	)

# Not needed since vapor shares include directory
# target_include_directories (common PUBLIC ${VAPOR_SOURCE_DIR}/include)
