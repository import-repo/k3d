SET(K3D_FREETYPE2_FOUND 0)

INCLUDE(K3DFindPkgConfig)
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
