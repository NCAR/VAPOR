set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (MACOSX_BUNDLE_GUI_IDENTIFIER testApp)
set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
set (INSTALL_BIN_DIR ".")


find_package(Qt5 REQUIRED COMPONENTS Core OpenGL Widgets Gui DBus Network)
find_package(HDF5 REQUIRED COMPONENTS C)
find_package(JPEG REQUIRED)
#find_package(OpenSSL REQUIRED COMPONENTS SSL Crypto)
#message(STATUS "OpenSSL Include: ${OPENSSL_INCLUDE_DIR}")
#message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
find_package(PROJ REQUIRED)
find_package(ZLIB REQUIRED)

#find_library(OSPRAY ospray HINTS ${THIRD_PARTY_DIR}/Ospray/lib REQUIRED)
find_library(FREETYPE freetype)
find_library(UDUNITS udunits2)
find_library(EXPAT expat)
find_library(SZLIB sz)
find_library(ASSIMP assimp)
find_library(GEOTIFF geotiff)
find_library(CURL curl)
find_library(LIBSSH ssh)
find_library(SQLite sqlite3)
find_library(TIFF tiff)
find_library(PNG png)
find_library(NETCDF netcdf)
find_library(CORESERVICES CoreServices)
find_library(FOUNDATION Foundation)
find_library(COCOA Cocoa)

add_executable (testApp MACOSX_BUNDLE testApp.cpp)

target_link_libraries(testApp ${CORESERVICES} ${FOUNDATION} ${COCOA} ${NETCDF} ${PNG} ${JPEG} ${TIFF} ${SQLite} ${LIBSSH} ${CURL} ${GEOTIFF} ${ASSIMP} ${SZLIB})
target_link_libraries(testApp ${EXPAT} ${UDUNITS} ${FREETYPE} ${OSPRAY})
target_link_libraries(testApp Qt5::Widgets Qt5::OpenGL Qt5::Core Qt5::Gui Qt5::DBus Qt5::Network)
target_link_libraries(testApp PROJ::proj ZLIB::ZLIB)# UDUNITS::UDUNITS)# SZLIB:SZLIB)
target_link_libraries(testApp ${HDF5_LIBRARIES} ${CURL_LIBRARIES})

#target_link_libraries(testApp PRIVATE ${HDF5_LIBRARIES})
target_include_directories(testApp PRIVATE ${HDF5_INCLUDE_DIRS})
#target_include_directories(testApp PRIVATE ${THIRD_PARTY_DIR}/Ospray/include/ospray)
#target_include_directories(testApp PRIVATE /usr/local/VAPOR-Deps/current/include/freetype2)
#target_include_directories(testApp PRIVATE ${THIRD_PARTY_INC_DIR})

#target_link_libraries( testApp common wasp render ${Python_LIBRARIES})
target_link_libraries( testApp common wasp vdc params flow osgl )#render )

install (
	TARGETS testApp
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT testAppGUI
	)
