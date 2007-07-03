SET(K3D_FREETYPE2_FOUND 0)

######################################################################
# Posix specific configuration

IF(UNIX)
	INCLUDE(FindPkgConfig)
	PKG_CHECK_MODULES(FREETYPE2 freetype2)

	IF(FREETYPE2_FOUND)
		SET(K3D_FREETYPE2_INCLUDE_DIRS
			${FREETYPE2_INCLUDE_DIRS}
			)

		SET(K3D_FREETYPE2_LIB_DIRS
			${FREETYPE2_LIBRARY_DIRS}
			)

		SET(K3D_FREETYPE2_LIBS
			${FREETYPE2_LIBRARIES}
			)

		SET(K3D_FREETYPE2_FOUND 1)
	ENDIF(FREETYPE2_FOUND)

ENDIF(UNIX)

IF(WIN32)
	FIND_PATH(K3D_FREETYPE2_CONFIG_INCLUDE_DIR ft2build.h
		c:/gtk/include
		${K3D_GTK_DIR}/include
		DOC "Directory where the freetype2 config files are located"
		)
	MARK_AS_ADVANCED(K3D_FREETYPE2_CONFIG_INCLUDE_DIR)

	FIND_PATH(K3D_FREETYPE2_INCLUDE_DIR freetype
		c:/gtk/include/freetype2
		${K3D_GTK_DIR}/include/freetype2
		DOC "Directory where the freetype2 header files are located"
		)
	MARK_AS_ADVANCED(K3D_FREETYPE2_INCLUDE_DIR)

	SET(K3D_FREETYPE2_LIB freetype CACHE STRING "")
	MARK_AS_ADVANCED(K3D_FREETYPE2_LIB)

	SET(K3D_FREETYPE2_INCLUDE_DIRS
		${K3D_FREETYPE2_CONFIG_INCLUDE_DIR}
		${K3D_FREETYPE2_INCLUDE_DIR}
		)

	SET(K3D_FREETYPE2_LIBS
		${K3D_FREETYPE2_LIB}
		)

	SET(K3D_FREETYPE2_FOUND 1)

ENDIF(WIN32)

