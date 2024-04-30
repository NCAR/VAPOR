#find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt5 REQUIRED COMPONENTS Core OpenGL Widgets Gui DBus Network)
add_executable (testApp testApp.cpp)
#add_executable (testApp MACOSX_BUNDLE testApp.cpp)
#set (INSTALL_BIN_DIR ".")

target_link_libraries ( testApp
        Qt5::Widgets Qt5::OpenGL Qt5::Core Qt5::Gui Qt5::DBus Qt5::Network
)

find_library (CORESERVICES CoreServices)
find_library (FOUNDATION Foundation)
find_library (COCOA Cocoa)
# target_link_libraries (vapor "-framework CoreServices")
target_link_libraries (testApp ${CORESERVICES} ${FOUNDATION} ${COCOA})

set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
#
#set (SRCS ${SRCS}
#    core_profile_attributes.mm
#    mac_darkmode.mm
#    mac_darkmode.h
#    )
#set (MACOSX_BUNDLE_GUI_IDENTIFIER testApp)
##set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}})
##set (MACOSX_BUNDLE_BUNDLE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_RC})
##set (MACOSX_BUNDLE_ICON_FILE VAPOR.icns)
#set (MACOSX_BUNDLE_ICON_FILE_PATH ${CMAKE_SOURCE_DIR}/share/images/VAPOR.icns)
#set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
##add_executable (testApp MACOSX_BUNDLE ${MACOSX_BUNDLE_ICON_FILE_PATH} ${SRCS} ${UIS})
#set (INSTALL_BIN_DIR ".")
#
#add_executable (testApp testApp.cpp)
##target_link_libraries (testApp common)
#target_link_libraries (testApp Qt::Core)
#
#find_library (CORESERVICES CoreServices)
#find_library (FOUNDATION Foundation)
#find_library (COCOA Cocoa)
## target_link_libraries (vapor "-framework CoreServices")
#target_link_libraries (testApp ${CORESERVICES} ${FOUNDATION} ${COCOA})

install (
	TARGETS testApp
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
