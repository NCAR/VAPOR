file (GLOB PNG_FILES *.png)

set (TMS_DIRS
	BigBlueMarble
	NaturalEarth
	)

set (TMS_FILES
	BigBlueMarble.tms
	NaturalEarth.tms
	)


install (
	FILES ${TMS_FILES} ${PNG_FILES}
	DESTINATION ${INSTALL_SHARE_DIR}/images
	COMPONENT Share
	)

install (
	DIRECTORY ${TMS_DIRS}
	DESTINATION ${INSTALL_SHARE_DIR}/images
	COMPONENT Share
	)
