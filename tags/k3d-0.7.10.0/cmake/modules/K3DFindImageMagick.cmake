SET(K3D_IMAGEMAGICK_FOUND 0)

INCLUDE(K3DFindPkgConfig)
PKG_CHECK_MODULES(IMAGEMAGICK ImageMagick++)

IF(IMAGEMAGICK_FOUND)
	SET(K3D_IMAGEMAGICK_INCLUDE_DIRS
		${IMAGEMAGICK_INCLUDE_DIRS}
		)

	SET(K3D_IMAGEMAGICK_LIB_DIRS
		${IMAGEMAGICK_LIBRARY_DIRS}
		)

	SET(K3D_IMAGEMAGICK_LIBS
		${IMAGEMAGICK_LIBRARIES}
		)

	SET(K3D_IMAGEMAGICK_FOUND 1)
ENDIF(IMAGEMAGICK_FOUND)
