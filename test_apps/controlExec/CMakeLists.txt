find_package(Qt4 REQUIRED)
set (CMAKE_AUTOUIC ON) # This needs to appear before adding sources to work properly
set (CMAKE_AUTOMOC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable (test_CE test_CE.cpp test_vizwin.cpp moc_test_vizwin.cpp)

target_link_libraries (test_CE common vdc wasp params render ${GLEW} Qt4::QtCore Qt4::QtGui Qt4::QtOpenGL jpeg tiff)
