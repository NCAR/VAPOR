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
	FileUtils.cpp
	ResourcePath.cpp
	LegacyVectorMath.cpp
	STLUtils.cpp
	VAssert.cpp
	Progress.cpp
	TMSUtils.cpp
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
    ${PROJECT_SOURCE_DIR}/include/vapor/debug.h
    ${PROJECT_SOURCE_DIR}/include/vapor/FileUtils.h
    ${PROJECT_SOURCE_DIR}/include/vapor/ResourcePath.h
    ${PROJECT_SOURCE_DIR}/include/vapor/LegacyVectorMath.h
    ${PROJECT_SOURCE_DIR}/include/vapor/STLUtils.h
    ${PROJECT_SOURCE_DIR}/include/vapor/NonCopyableMixin.h
	${PROJECT_SOURCE_DIR}/include/vapor/VAssert.h
	${PROJECT_SOURCE_DIR}/include/vapor/Progress.h
	${PROJECT_SOURCE_DIR}/include/vapor/TMSUtils.h
	${PROJECT_SOURCE_DIR}/include/vapor/OpenMPSupport.h
)

add_library (common SHARED ${SRC} ${HEADERS})

if (APPLE)
	find_library (COREFOUNDATION CoreFoundation)
	target_link_libraries (common ${COREFOUNDATION})
endif()

if (NOT WIN32)
	target_link_libraries (common pthread)
	add_definitions (-DENABLE_THREADS)
endif()

add_definitions (-DCOMMON_EXPORTS)

# This is necessary because we include external header files in our headers
target_include_directories (common
	PUBLIC "${PROJECT_SOURCE_DIR}/include"
	PUBLIC "${THIRD_PARTY_INC_DIR}"
	PUBLIC "${THIRD_PARTY_INC_DIR}/freetype2"
	)

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
