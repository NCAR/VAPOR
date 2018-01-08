add_executable (test_dccompare test_dccompare.cpp)

target_link_libraries (test_dccompare common vdc)

install (
	TARGETS test_dccompare
	DESTINATION ${INSTALL_BIN_DIR}
	COMPONENT Utilites
	)
